## <center>RISC-V CPU模拟器设计报告</center>

### <center>160001896 金庆哲</center>



### 实验开发环境

本次实验主要在macOS上进行开发，但是对于riscv-64i可执行文件需要在64位的ubuntu中使用riscv-64-unknown-elf-gcc进行编译生成。



### 设计概述

代码结构组织如下图

![image-20190417113619068](https://ws3.sinaimg.cn/large/006tNc79ly1g25h9ftykej31cc0mm77h.jpg)

* Machine.h

  定义了后面需要用到的很多常量，如寄存器个数，栈基址，栈大小，寄存器名字对应的编号，指令对应编号，不同类型的opcode对应的编号等，简化了编程难度

* MM

  MM中主要是定义了一些和内存访问相关的函数，包括读取elf进内存，以及一些读内存和写内存的指令

* Simulator

  Simulator是模拟器的核心部分，将模拟器定义为一个class，里面定义了许多要用到的变量，参数等，如寄存器、流水线控制变量等，最核心的内容是流水线相关的部分。

  对于四个流水线寄存器的定义由模板中启发，每个寄存器都需要定义两份，一份用来这个周期的执行，一份用来输入到下个周期，从而模拟五级流水线。

  而对于五个阶段的执行，由于我们在单cpu上只能顺序，所以就按照fetch、decode、execute、memory、writeBack的顺序执行，每个阶段定义为一个函数，输入就是上个周期更新后的该阶段对应的流水线寄存器，在其他的.cc文件中分别实现以缩减单个代码文件的长度，便于调试和分析。

  使用其中的Run函数执行模拟过程，按顺序执行五个周期的函数，每个周期执行完五阶段，获得四个更新后的流水线寄存器后再统一对流水线寄存器进行更新，然后循环。

  在此之外，Simulator的构造函数还需要负责栈空间的初始化，寄存器的初始化（SP要设置成栈基址）

* main

  main函数就是模拟的主过程，最主要的是要分析用户参数的输入，设置相关的变量，初始化内存空间，调用函数读取elf进入内存，然后使用simulator的run函数来进行模拟。



### 具体设计和实现

* 可执行文件相关

  这里经人推荐，使用了一个很方便的开源代码elfio，可以通过封装好的方法来方便的读取elf中的内容，我们按位读进内存就可以了。

  这里将32位内存组织为多级页表（大部分程序32位完全足够），能够表示2^48^的内存空间（具体而言是一个三位数组），每次我们从elf文件中读进内容是，如果发现这一页没有申请过（也就是为NULL），就先将这页初始化为0，然后在读进相关内容。

  逻辑如下，这里的GET_FPN等是定义在MM.h中的宏

  ~~~C
   Elf_Half seg_num = elf_reader->segments.size();
      while(seg_num--) {
          segment* seg = elf_reader->segments[seg_num];
          uint32_t memory_size = seg->get_memory_size();
          uint32_t v_addr = (uint32_t)seg->get_virtual_address();
          uint32_t file_size = seg->get_file_size();
          for (uint32_t x = 0; x < memory_size; x++) {
              uint32_t ad = x + v_addr;
              if (!check_Page(ad))
                  get_Page(ad);
              uint32_t first = GET_FPN(ad);
              uint32_t second = GET_SPN(ad);
              uint32_t offset = GET_OFF(ad);
              if (x < file_size)
                  mem[first][second][offset] = seg->get_data()[x];
              else {
                  mem[first][second][offset] = 0;
              }
          }        
  ~~~

  在main中需要通过elfio的get_entry()函数来获得起始PC的值。

* 流水线具体处理过程

  控制信号定义在Simulator.h中的四个流水线寄存器中。

  每个寄存器中都需要设置一个bool bubble代表这个周期中是否被bubble，如果被bubble的话在对应的函数中的操作就是将下一个周期下一个阶段寄存器的bubble也设为true，让这个bubble向下传播直到writeBack结束，其实也就是代表撤销了这一个指令的操作。而在Run最开始的时候，四个寄存器的bubble值都需要设置为true，因为一开始流水线是空的，五个周期后才能充满整个流水线。

  * Fetch

    Fetch阶段要做的就是从PC值指向的内存位置处拿到长度为4的指令，然后将PC加4

    ~~~C
    struct Fetch_to_Decode
        {
          bool bubble;
          unsigned instruction;
          unsigned long long PC;
        } 
    ~~~

    从Fetch到Decode，只需要传一个指令instruction用来解码（riscv64i的指令固定为4字节长，所以定义为unsigned），以及一个PC值用来后面的一些跳转操作。

  * Decode

    Decode阶段做的事情逻辑上也十分简单，通过位运算算出指令中的opcode来确定指令类型，再根据不同指令类型的格式来获得指令的参数（如rs1、rs2、rd、imm等）。要注意的一点是分支指令的预测需要在这里进行，如果发现指令的opcode代表它是分支指令，就使用predictBranch函数来预测结果

    ~~~C
    bool
    Simulator::predictBranch()
    {
    	if (!strategy) {
    		return true;
    	}
    	
    	else if (strategy == 1) {
    		return false;
    	}
    	return true;
    }
    ~~~

    这里实现了always take和never take两种，如果预测结果是选择跳转，就需要在fdRegNew中插入一个bubble来撤销这个周期fetch到的错误指令，然后将Simulator中的变量PC_taken和PC_not_taken设置成选择的PC和没有选择的PC（若是选择跳转，前者就是PC+4，后者就是PC+offset，否则反过来），而对于分支预测的更新需要在每个周期结束并更新寄存器之后（会有一些判断，在后面详细阐述）

    ~~~C
       struct Decode_to_Execute
        {
          bool bubble;
          int op_type;
          long long offset;
          unsigned long long PC;
          bool takeBranch;
          int imm;
          int rs1, rs2;
          int rd;
          int64_t op1, op2;
        } 
    ~~~

    从decode传送到execute的流水线寄存器中的信息如上，op_type是指令类型，显然是必要的。一些跳转指令会需要offset进行运算（其实可以放到op中，但是这样可以更好的对照指令表来编写代码）；PC值用来计算跳转地址；takebranch代表预测的结果，在execute阶段如果发现预测错误了要进行相应的处理；imm是立即数，rs1、rs2、rd的默认值都为-1，所以在后面可以通过检查rd的值来判断指令是否需要写寄存器。而op1、op2则是传给execute的操作数的值，如果后面发生了需要数据前送的情况要改变的就是这两个值。

  * Execute

    Execute阶段要根据寄存器中得到的操作类型来进行相应的操作，并算出对应的结果向后继续传递。

    这里比较特殊的就是跳转指令和分支指令，如果发现是跳转指令，就需要将PC值直接置成跳转目的地（因为没有过多的特殊情况，所以不需要在寄存器更新后再更新），同时需要撤销现在已经取进来的两个错误指令，也就是向下周期FtoD的reg和DtoE的reg中插入bubble。

    还有就是一些需要数据前送的判断，对于分支预测结果错误等的处理，这些放在后面的部分中详细讨论。

    ~~~C
     struct Execute_to_Memory
        {
          bool bubble;
          int64_t op2, out;
          bool wMem, rMem; // read or write signals
          bool signExt; // if need to use sign extension
          int rd;
          int memsize;
        } 
    ~~~

    流水线寄存器的内容如上，op2是store需要的要保存的数据，out是计算结果，wMem和rMem代表是否是读写内存的操作，signExt代表后面load值之后是否需要进行富豪扩展，rd代表目的寄存器（若为-1则说明不需要写寄存器），memsize是读写内存的数据长度

  * Memory

    Memory要做的就是访问内存的操作，若wMem为true，就写所需长度的内存内容，若rMem为true，就读取所需长度的内容，这两个操作都用函数封装了起来

    ~~~C
    void
    StoreBySize(uint32_t addr, uint64_t val, int memsize)
    {
        if (!check_Page(addr)) {
            printf("Invalid address!\n");
        }
        uint32_t first_id = GET_FPN(addr);
        uint32_t second_id = GET_SPN(addr);
        uint32_t offset = GET_OFF(addr);
        memcpy(&mem[first_id][second_id][offset], &val, memsize);
    }
    
    uint64_t
    LoadBySize(uint32_t addr, int memsize)
    {
        uint64_t x = 0;
        for (int i = 0; i < memsize; i++) {
            x += (((uint64_t)getB(addr + i)) << (i * 8));
        }
        return x;
    }
    ~~~

    如果都不是，那么就只需要判断一下是否需要数据前送（注意避免数据覆盖）

    ~~~C
    struct Memory_to_WB
        {
          bool bubble;
          int64_t out;
          int rd;
        } 
    ~~~

    寄存器只需要传一个out值和一个目的寄存器给writeBack阶段即可

  * WriteBack

    writeBack阶段的操作最为简单，检查rd是否为-1，如果不是就将数据写回寄存器，然后判断数据前送的情况

  我们按顺序执行这五个阶段，更新寄存器，进行一些数据冒险和PC更新的判断，然后循环

* 各种冒险的处理

  * 数据冒险

    对于数据冒险的处理基本是按照《计算机体系结构/硬件软件接口》这本书中的逻辑来实现的，但是因为我们的控制信号设计以及实际的执行流程和现实中的流水线处理器有着一定的区别，所以需要进行一些修改。我们能够发现数据冒险的阶段有Execute、Memory和WriteBack

    * Execute

      在Execute阶段可以发现数据冒险的情况，这里分为两种。

      * 需要stall的情况

        判断情况如下

        ~~~C
         if (emRegNew.rMem && (deRegNew.rs1 == rd || deRegNew.rs2 == rd))
        ~~~

        也就是读内存的目的寄存器和解码阶段读的源寄存器相同，无法直接通过数据前送的方式解决（逻辑上会变成数据后送，不实际），根据书上的逻辑我们需要在Execute阶段插入一个气泡，同时让Fetch阶段和Decode阶段重复上个周期的内容。

        插入气泡是容易的，让Decode阶段重复上个周期的内容可以通过在执行完五个阶段后不更新FtoD的Reg来实现，而让Fetch阶段重复上个周期的内容就需要将PC减四、同时在fetch阶段进行判断，如果被stall，就将PC加4并返回。

        这里在Simulator中定义变量stall，如果在execute阶段发现了这种情况，就将stall置为true，而在五个阶段执行结束后，判断stall来决定是否更新FtoD的Reg，以及PC是否减四

        ~~~C
        if (stall) {
        			PC -= 4;
        			if (fdReg.bubble)
        				fdReg.bubble = false;
        		}
        		else {
        			fdReg = fdRegNew;
        		}
        ~~~

        这里对于bubble的判断是因为decode阶段如果是选择跳转的branch操作，那么fdReg.bubble会被置为true从而撤销了这个指令，但我们想要的是让decode阶段重复上个周期的内容，因此需要让它重新预测一次分支，下个周期再考虑是否bubble。

        而将stall重新设置为false的时机在fetch中，它是我们为了stall所最后做的一个操作。

      * 不需要stall的情况

        ~~~C
        if (!emRegNew.rMem && (rd != -1) && (rd != 0)) {
                if (deRegNew.rs1 == rd) {
                    deRegNew.op1 = out;
                    exeWBdest = rd;
                }
            	else if (deRegNew.rs2 == rd)
                    ....
        }
                
        ~~~

        这种情况就是不读内存，但是也有目的寄存器和decode阶段冲突的情况，那么我们只需要把out值传到对应的op中即可。

        要注意的是这里在Simulator类中定义了一个exeWBdest，如果为-1就说明execute阶段没有进行前送操作，这是为了避免多个阶段同时向一个寄存器前送数据，导致数据覆盖，如果出现这种情况，execute的值是最优先的（因为指令离冲突目标最近，值是最新的），然后是memory，最后才是writeBack。

    * Memory

      Memory阶段的数据冒险是一定不需要stall的，那么逻辑就和Execute中的第二种相同

      ~~~C
      if (rd != 0 && rd != -1) {
              if (rd == deRegNew.rs1) {
                  if (exeWBdest != rd) {
                      memWBdest = rd;
                      deRegNew.op1 = mwRegNew.out;
                  }
              }
          	else if ...
      }
      ~~~

      只是要多判断是否有覆盖问题，这里memWBdest的意义和exeWBdest完全类似，用来在后面判断覆盖。

    * WriteBack

      事实上书上表示WriteBack阶段并不会有数据冒险的问题，这是因为在现实的流水线中五个阶段是同时进行的，那么在decode取出数据的时候，WriteBack已经将值写回了，不会出现错误。但是我们这里是顺序执行的，所以decode的值还是会出现问题，因此需要判断一下（注意覆盖问题）

      ~~~C
      if (rd != -1 && rd != 0) {
              if (deRegNew.rs1 == rd) {
                  //avoid overwritting, least priority
                  if (rd != exeWBdest && rd != memWBdest) {
                      deRegNew.op1 = mwReg.out;
                  }
              }
          	else if ....
      }
      ~~~

  * 控制冒险

    这里的控制冒险主要是在分支预测上（跳转直接就跳了），分支预测的逻辑上面有提到过，更新时机是在五个阶段执行结束后

    ~~~C
    if (deReg.takeBranch && !stall && !deReg.bubble){
    			PC = PC_taken;
    		}
    ~~~

    如果说我们选择跳转，并且没有stall的情况，并且deReg.bubble也为false，就将PC设置为选择的分支地址。

    不能有stall的原因是因为stall的时候我们需要重复执行fetch阶段，如果更新PC就会导致错误，stall机制会重新运行一次decode，那么下个周期我们就可以继续正常的预测分支。

    deReg.bubble不能为true的原因是为了防止覆盖掉jump的跳转（这里显然jump更加优先）

    在预测后下一周期的execute阶段，我们可以通过运算的结果来检查出预测结果的正确与否，如果错误的话，就需要向FtoD和DtoE阶段插入两个bubble来消除错误取入的指令，并且将PC值更新为PC_not_taken，也就是修正PC值。

* 系统调用的处理

  在execute阶段发现操作类型为系统调用后，会调用我自己编写的Simulator::syscall函数，接收的输入就是a0和a7寄存器的值，对a7寄存器的值进行判断来确定系统调用的类型，从而处理相应的函数。我们的测试程序中所使用的系统调用只有exit（程序结束后默认使用），调用号为93，这里的行为是打印一些统计数据后调用exit(0)来结束程序。

  同时在我自己编写的测试程序中还包含了打印字符和打印整数的系统调用

  ~~~C
  switch (type)
      {
          case 0:
          printf("%d", (int)arg);
          break;
          case 1:
          printf("%c", char(arg));
          break;
          case 93:
          printf("Simulator Exiting!\n");
          print....
          exit(0);
          default:
              printf("Syscall %d Unknown!!\n", type);
              exit(-1);
      }
  ~~~

* 性能计数的处理

  所有的性能计数的值都存放在Simulator类中

  对于执行周期，在每次执行完五个阶段后，让周期加一

  对于动态指令数，在execute阶段进行计数（如果不bubble就加一，所有正确执行的操作都会经过execute阶段，否则可能被撤销或者停顿）

  对于各种冒险造成的停顿，根据影响的阶段数来计算。比如memload造成的stall，会导致fetch和decode停顿一周期，然后插入一个气泡到execute阶段，这个气泡会影响三个阶段，所以这个stall就会让memLoadHazard的值加5；而如果是指令预测错误，向decode和execute阶段插入两个气泡，这两个气泡最后一共会影响7个阶段，所以会让controlHazard的值加7

  对于分支预测，在execute阶段中判断，如果算出预测正确，那么正确数就加一，否则错误数加一，最后可以算出一个预测正确率

* 调试接口

  * 程序使用cmake生成了Makefile文件，使用 “make"来编译文件

  * 所有生成的riscv可执行文件都放在elfs文件夹中

  * 测试指令格式如下

    ~~~C
    ./Simulator "filestr" -p -s -q "BranchStrategy"
    ~~~

    -p代表打印执行信息，-s代表单步调试，-q代表选择分支预测策略（默认为always take），-q后面可以加“Always”或者“Never”

    一个实例执行指令

    ~~~C
    ./Simulator ./elfs/ack.riscv -p -q Never
    ~~~



### 功能测试和性能评测

这里只放出其中一个程序的运行结果，后面的用表格来直观展示

![image-20190417145849630](https://ws4.sinaimg.cn/large/006tNc79ly1g25n45gieaj30v80a6myl.jpg)

* 动态指令数

  | simple-function.c | add.c | n\!.c | qsort.c | mul-div.c |
  | ----------------- | ----- | ----- | ------- | --------- |
  | 920               | 910   | 1129  | 19484   | 935       |

  可以看到，动态指令数和程序的复杂程度是正相关的

* 周期数

  | simple-function.c | add.c | n\!.c | qsort.c | mul-div.c |
  | ----------------- | ----- | ----- | ------- | --------- |
  | 1309              | 1295  | 1636  | 26218   | 1320      |

* CPI

  | simple-function.c | add.c  | n\!.c  | qsort.c | mul-div.c |
  | ----------------- | ------ | ------ | ------- | --------- |
  | 1.4228            | 1.4231 | 1.4491 | 1.3456  | 1.4118    |

  可以看到，qsort的cpi要显著小于其他四个程序，这可能是因为他的指令更加密集，更适合放入流水线中“并行”处理

* 冒险停顿（数值代表轮空的阶段数）（这里分支预测策略是Always take）

  * 控制冒险

    | simple-function.c | add.c | n\!.c | qsort.c | mul-div.c |
    | ----------------- | ----- | ----- | ------- | --------- |
    | 1545              | 1531  | 2153  | 16763   | 1531      |

    由于qsort中大量的while和if操作，分支预测次数很多，所以因此产生的控制冒险产生的停顿数显著的多余其他几个主要是在做计算工作的代码

  * 数据冒险

    | simple-function.c | add.c | n\!.c | qsort.c | mul-div.c |
    | ----------------- | ----- | ----- | ------- | --------- |
    | 290               | 290   | 170   | 15870   | 290       |

    可以看到，出了qsort之外，剩下四个代码的数据冒险停顿远少于控制冒险产生的停顿，而qsort则是量级几乎相同。可能是由于qsort会不断的递归调用qsort这个函数，而函数的运行参数需要从栈中读取，从而会有很多的访存操作，而产生数据冒险。



### 对于之前实现代码的运行

* ack

  ![image-20190417155233523](https://ws2.sinaimg.cn/large/006tNc79ly1g25oo2axfuj30v80mmgp7.jpg)

  可以看到，这里面对于ackermann函数结果的计算是正确的（这里的输出使用自己定义的函数）

* Quick(完全类似测试程序中的quicksort，就不显示了）

* matmul

  ![image-20190417155435353](https://ws4.sinaimg.cn/large/006tNc79ly1g25oq6btmhj30vk0aegn6.jpg)

  这是一个运算量很大的文件，可以稍微证明我们模拟器的鲁棒性



### 分支预测性能

* Always Take

  | ack    | mat    | quick  |
  | ------ | ------ | ------ |
  | 0.6525 | 0.7056 | 0.5133 |

* Never Take

  | Ack    | mat    | quick  |
  | ------ | ------ | ------ |
  | 0.3475 | 0.2944 | 0.4888 |

在for循环中，大部分时间的分支都是应该选择跳转的，因此如果在程序中存在大量的循环语句，Always Take的准确率就会很高（Never take就会越低），而quick sort中并没有很多的循环语句，而是有着很多的while和函数调用，因此Always Take的准确率就要低于其他两个程序



### 遇到的问题

* 在编写此次的lab时，遇到最大的坑就是有关数据类型的错误，哪里应该是unsigned、哪里应该是int、哪里应该是int64_t都需要进行慎重的考虑，一旦一个小地方写错了，可能会导致出现一些难以调试的bug，这种bug是很难通过任何方式检查出来的，因为这个问题而困扰了很久。尤其是要注意根据spec手册中的数据类型定义来模拟操作（比如一些imm为符号扩展等）
* 虽然教材中有很详细的冒险处理逻辑，但是我们必须注意到我们的实现和现实中流水线的差别，从而设计出一个符合自己逻辑的处理机制
* 要尽量减少流水线寄存器中控制信号的数量，这样便于管理和削减冗余操作（得益于riscvi指令的简单）



### 意见和建议

* 可以试着提供一些方便使用的开源代码（比如elfio就简化了很多操作）
* simple green card只有一部分的指令，很大一部分指令还是要去spec和isa-list中看，但是给的指导手册中只说了参考simple green card，可能会产生一部分误导效果