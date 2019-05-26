## 计算机体系结构实习LAB4

#### 1600012896 金庆哲



### 一、使用SIMD指令对应用程序进行优化

#### 实现方式

在第一部分中，我们的任务是对yuv图像进行处理，主要分为两步，先将yuv图像转为RGB格式，然后对RGB格式的图像进行alpha混合，最后将混合后的RGB图像转换为yuv图像并保存，而我们需要用不同的方式来编写这段代码。在这里使用Intel的Intrinsic函数来编写SIMD部分的代码（和汇编逻辑差别不是很大，但是方便和C语言一同编写）

* 不使用SIMD

  在实习指导中有RGB和yuv互转的浮点版本公式，我们按照公式来转换即可。同时为了后面更好的使用SIMD指令，还自行上网查找了整数版本的互转公式并加以实现。

  不管是哪个版本的公式，其本质都是类似于向量点积的操作，我们可以利用这一点来对其进行并行化。

  这里要注意的一点是，在读取图像的yuv值时，要将其转化成unsigned格式，否则会读到负数从而影响后面的计算，导致整个图片十分的绿。
  
* MMX

  MMX指令集的特点是使用64位的寄存器，同时只支持整数的运算操作（这也是为什么我们需要找到整数版本的转换公式）

  一个转换操作的格式如下

  ~~~C
  int R = (298 * (y - 16) + 411 * (v - 128) + 32) >> 8;
  int B = (298 * (y - 16) + 519 * (u - 128) + 83) >> 8;
  int G = (298 * (y - 16) - 101 * (u - 128) - 211 * (v - 128)  - 429) >> 8;
  ~~~

  在这里，y、u、v都是可以按照16位的short类型来保存，因此一个mmx的寄存器中可以保存4个操作数

  那么我们最理想的情况肯定是让每次运算都用到寄存器中的所有操作数位置，也即一次让四个操作数都参与运算。

  我们要利用的核心指令是_mm_madd_pi16( __ m64a, __m64b)，其逻辑如下

  ~~~
  FOR j := 0 to 1
  	i := j*32
  	dst[i+31:i] := a[i+31:i+16]*b[i+31:i+16] + a[i+15:i]*b[i+15:i]
  ~~~

  这里有两种思路

  * 第一种是每次只计算一个像素值（R或G或B）

    ~~~C
    __m64  part1 = _mm_set_pi16(v, u, y, 1);
    __m64  op = _mm_set_pi16(128, 128, 16, 0);
    __m64 eight = _mm_set_pi32(0, 8);
    part1 = _mm_sub_pi16(part1, op);
    op = _mm_set_pi16(411, 0, 298, 32);
    part1 = _mm_madd_pi16(part1, op);
    int R = (int(_mm_cvtm64_si64(part1) >> 32) + int(_mm_cvtm64_si64(part1) & 0x00000000ffffffff)) >> 8;
    ~~~

    逻辑如上，优点是每次运算都用到了四个操作数，但是缺点是需要对每个像素都进行两次从mmx数据类型到int的转换，并且还要对转换后的数字进行一些位移和与操作来获取对应位置的值，最后还需要进行一次八位的右移（而这个操作是运算R、G、B都要用到的，理应可以并行化）

  * 第二种是同时计算多个像素值

    我们将像素的运算分成两部分

    一部分是op1 * y + op2 * v， 另一部分是op3 * u + op4 * v， 然后每次madd就是对于两个像素计算过程的一部分的运算，这样依旧可以让每次运算都用到四个操作数，同时可以提高后面右移操作和alpha乘法操作的并行性（因为每次都是对于两个像素值操作）

    ~~~C
    part1 = _mm_set_pi16(y, u, y, u);
    op = _mm_set_pi16(16, 128, 16, 128);
    part1 = _mm_sub_pi16(part1, op);
    op = _mm_set_pi16(298, -101, 298, 519);
    part1 = _mm_madd_pi16(part1, op);
    ~~~

    比如计算第一部分的结果就是上面的这段代码，每次操作都是同步计算两个像素值，进行完这些计算后part1中的内容应该是(Bpart1, Gpart1)，然后我们用同样的方式计算得到(Bpart2, Gpart2)，然后加起来就得到了(B, G)

  显然，我们应该尽可能的一次性计算多个像素值，但是受限于mmx的寄存器大小，我们一次最多只能计算两个像素值，因此我们可以用第一种思路计算R，用第二种思路计算G和B

  而将RGB转回yuv时逻辑完全类似，因为都是四个操作数的点积操作，所以我们也是用第一种思路计算y，第二种思路计算u和v，最后保存起来即可

* SSE2

  SSE相比于MMX来说，主要是增加了对于浮点数的操作功能，同时将寄存器扩展到了128位。但是在SSE2中，对于单精度浮点数的支持很少（难以对点积形式的计算进行并行优化），而一个寄存器只能存放两个双精度浮点数，并且也没有对于类似点积操作的支持，因此我们还是考虑使用整数形式的转换方法（因为SSE2中保留了madd）

  那么我们的逻辑就和mmx完全类似了，不过这次我们能在寄存器中存放16个short，8个int，也就是说我们可以完全使用上面所说的第二种利用madd的逻辑（使用第一种会浪费很多并行度），但是我们不可避免的会浪费32位的位置，因为我们一共只需要进行三个像素值的计算。

  代码完全类似上面的第二种逻辑，计算得到一个存放(0, R, G, B)的寄存器，然后取出R，G，B并重新计算yuv

  另一个可以在mmx的代码上提升的操作是对于SIMD数据类型向int的转换。在mmx中我们只能直接转换成int64，然后对64位整数进行一些位移和与操作才能得到int，而SSE2中可以直接用一个_mm_extract_epi16指令来完成

* AVX

  AVX相比于前两种SIMD指令集，增加了许多浮点运算操作，并且将寄存器扩展到了256位

  最关键的一点是出现了一个完美契合我们运算逻辑的指令_mm256_dp_ps（事实上SSE4中就出现了，可惜我们只能用SSE2来编写）

  这个指令的逻辑如下

  ~~~C
  DEFINE DP(a[127:0], b[127:0], imm8[7:0]) {
  	FOR j := 0 to 3
  		i := j*32
  		IF imm8[(4+j)%8]
  			temp[i+31:i] := a[i+31:i] * b[i+31:i]
  		ELSE
  			temp[i+31:i] := 0
  		FI
  	ENDFOR
  	
  	sum[31:0] := (temp[127:96] + temp[95:64]) + (temp[63:32] + temp[31:0])
  	
  	FOR j := 0 to 3
  		i := j*32
  		IF imm8[j%8]
  			tmpdst[i+31:i] := sum[31:0]
  		ELSE
  			tmpdst[i+31:i] := 0
  		FI
  	ENDFOR
  	RETURN tmpdst[127:0]
  }
  
  dst[127:0] := DP(a[127:0], b[127:0], imm8[7:0])
  dst[255:128] := DP(a[255:128], b[255:128], imm8[7:0])
  dst[MAX:256] := 0
  ~~~

  也就是说，我们可以让a包含4个操作变量，b包含4个操作常数，然后合理的设置imm8，从而使得最后的dst中的特定位置保存点积的运算结果，这样仅仅使用dp_ps一个指令就可以算出一个像素值，同时AVX中的dp_ps相当于同时对寄存器的高128位和低128位进行相同的操作，那么我们可以一次性算出两个操作常数相同的像素值（两个R、两个G、两个B），因此我们可以考虑每次循环迭代中计算两个yuv值对应的RGB

  比如我们设置imm8，通过三次dp_ps获得了(0, 0, 0, R, 0 ,0 , 0, R)， (0, 0, G, 0, 0, 0, G, 0)，(0, B, 0, 0, 0, B, 0, 0)，然后将三者相加即可得到(0, B, G, R, 0, B, G, R)，然后可以继续相同的逻辑来获得新的yuv值



#### 编译运行方式

* 编译指令

  ~~~C
  g++ image.cc -O2 -mavx2 -o image
  ~~~

  这里需要加入mavx2来支持avx指令，同时使用O2级别的优化

* 运行指令

  ~~~C
  ./image -i <指令集名称> -o
  ~~~

  指令集名称包括avx，sse，float，int，mmx

  加入-o会输出yuv图像



#### 运行结果

首先来看下运行所得到的图像，分别为alpha=120和alpha=255的情况

* int

  ![1558500606806](E:\Users\Administrator\PycharmProjects\RiscV-Simulator\yuv\assets\1558500606806.png)

  ![1558500571611](E:\Users\Administrator\PycharmProjects\RiscV-Simulator\yuv\assets\1558500571611.png)

* float

  ![1558500740768](E:\Users\Administrator\PycharmProjects\RiscV-Simulator\yuv\assets\1558500740768.png)

  ![1558500772135](E:\Users\Administrator\PycharmProjects\RiscV-Simulator\yuv\assets\1558500772135.png)

* MMX
  ![1558500807575](E:\Users\Administrator\PycharmProjects\RiscV-Simulator\yuv\assets\1558500807575.png)
  
  ![1558500891187](E:\Users\Administrator\PycharmProjects\RiscV-Simulator\yuv\assets\1558500891187.png)

* SSE2

  ![1558500930182](E:\Users\Administrator\PycharmProjects\RiscV-Simulator\yuv\assets\1558500930182.png)

  ![1558500957893](E:\Users\Administrator\PycharmProjects\RiscV-Simulator\yuv\assets\1558500957893.png)

* AVX

  ![1558501005050](E:\Users\Administrator\PycharmProjects\RiscV-Simulator\yuv\assets\1558501005050.png)

  ![1558501033671](E:\Users\Administrator\PycharmProjects\RiscV-Simulator\yuv\assets\1558501033671.png)

可以看出，五种方法所得到的图像基本看不出任何的区别

然后再来看下运行效率，这里的时间是运行五次得到的平均值，这里的时间也包括了从矩阵中获取数据和得到整数存放到矩阵中的过程

| **Int**   | 3.20791s     |
| --------- | ------------ |
| **Float** | **4.23854s** |
| **MMX**   | **4.06685s** |
| **SSE2**  | **2.28975s** |
| **AVX**   | **1.13482s** |

首先可以看到整数的转换操作要快于浮点的转换操作，这是显然的，因为整数运算要快于浮点运算

这里我们应该将Int，MMX，SSE2一同分析，Float和AVX一同分析，因为前三者都用了整数的转换公式，后二者使用的是浮点的转换公式

出人意料的是，虽然我们在编写MMX函数的过程中确实利用了运算的并行性，但是其处理时间却要比不使用SIMD的整数操作慢了不少，看似出人意料，其实也是理所当然。因为不论在使用哪种SIMD指令集的时候，我们都需要使用一些将SIMD数据格式转换为整数的操作，而事实上查看手册可以发现，这些指令的代价都是比较大的，在MMX中还需要对获得的整数进行一些冗余的处理，更增加了操作时间，观察所写代码就可以发现和运算无关的转换指令十分之多，其代价远大于增加少许并行度所带来的优势

而SSE2比MMX快了接近一倍，虽然逻辑类似，但是多利用了一些并行度，这说明提高并行度确实可以加速运算。但事实上SSE2一次计算三个像素，而MMX一次可以计算两个，并不应该有一倍的提升，这多余的提升是来自于使用extract操作来替换之前的cvtsi64，因为extract不需要再进行多余的位移和与操作，大大减少了操作数量

最快的是使用AVX指令集，比逻辑相同的不使用SIMD的浮点版本快了接近四倍，这是理所当然的。因为我们用一条指令计算了四维向量的点积操作，如果不需要考虑将SIMD数据转换为int的损失，单纯的计算速度提升会快更多

因此对于这个函数来说，使用AVX指令集进行优化是最优解，其包含的dp_ps指令最契合函数的逻辑



### 二、设计自定义扩展指令对SIMD应用优化并分析

我们考虑针对上面的图像处理函数设计一个扩展指令集，为了操作的效率，应该使用整数操作的那套方案。

共有八个256位的寄存器，记为jmm0-jmm7

而我们指令的内容可以很大程度上借鉴sse2，而最关键的指令可以仿照avx中的dp_ps来设计一个整数版本

指令的写法最左边为最高位，最右面为最低位

通过最高4位的opcode来标识指令种类下面的Unpack，Pack，算术运算及jmovr都属于R型指令，opcode域为0001；jmovdqa为M型指令，opcode域为0010；数据类型转换指令为C型指令，opcode域为0011



### Unpack

| opcode | funct4 | functype | r2    | r1    | dst   | zeroes   |
| ------ | ------ | -------- | ----- | ----- | ----- | -------- |
| 0001   | 4bits  | 0000     | 3bits | 3bits | 3bits | 11bits 0 |



* junpckhbw dst, r1, r2

  * 编码

    funct4 = 0x1

  * 语义

    将r1和r2中的高半部分的8位整数unpack然后存储到dst

    即：dst[7:0] = r1[135:128] ;
  
    ​	    dst[15:8] = r2[135:128];
  
    ​		dst[23:16] = r1[143:136]; ....
  
* junpckhwd dst, r1, r2

  * 编码

    funct4 = 0x2

  * 语义

    将r1和r2中的高半部分的16位整数unpack并存储到dst，和上面类似

* junpckhdq dst, r1, r2

  * 编码

    funct4 = 0x3

  * 语义

    将r1和r2中的高半部分的32位整数unpack并存储到dst，和上面类似

* junpcklbw dst, r1, r2

  * 编码

    funct4 = 0x4

  * 语义

    将r1和r2中的低半部分的8位整数unpack并存储到dst

    即：dst[7:0] = r1[7:0];

    ​		dst[15:8] = r2[7:0];

    ​		dst[23:16] = r1[15:8]; ....

* junpcklwd dst, r1, r2

  * 编码

    funct4 = 0x5

  * 语义

    将r1和r2中的低半部分的16位整数unpack并存储到dst，和上面类似

* junpckldq dst, r1, r2

  * 编码

    funct4 = 0x6
  
  * 语义
  
    将r1和r2中的低半部分的32位整数unpack并存储到dst，和上面蕾丝

### Pack
| opcode | funct4 | functype | r2    | r1    | dst   | zeroes   |
| ------ | ------ | -------- | ----- | ----- | ----- | -------- |
| 0001   | 4bits  | 0001     | 3bits | 3bits | 3bits | 11bits 0 |


* jpacksswb dst, r1, r2

  * 编码

    funct4 = 0x1

  * 语义

    将r1和r2中的16位整数使用有符号饱和转换以8位整数的形式保存在dst中

    即：dst[7:0] = Saturate_Int16_To_Int8 (r1[15:0])

    ​		dst[15:8] = Saturate_Int16_To_Int8 (r1[31:16])

    ​		...

    ​		dst[127:120] = Saturate_Int16_To_Int8 (r1[255:240])

    ​		dst[135:128] = Saturate_Int16_To_Int8(r2[15:0])

    ​		...

* jpackssdq dst, r1, r2

  * 编码

    funct4 = 0x2

  * 语义

    将r1和r2中的32位整数使用有符号饱和转换以16位整数的形式保存在dst中

    和上面类似

* jpackssqdq dst, r1, r2

  * 编码

    funct4 = 0x3

  * 语义

    将r1和r2中的64位整数使用有符号饱和转换以32位整数的形式保存在dst中

    和上面类似

* jpackuswb dst, r1, r2

  * 编码

    funct4 = 0x4

  * 语义

    将r1和r2中的16位整数使用无符号饱和转换以8位无符号整数的形式保存在dst中

    即：dst[7:0] = Saturate_Int16_To_UInt8 (r1[15:0])

    ​		dst[15:8] = Saturate_Int16_To_UInt8 (r1[31:16])

    ​		...

    ​		dst[127:120] = Saturate_Int16_To_UInt8 (r1[255:240])

    ​		dst[135:128] = Saturate_Int16_To_UInt8(r2[15:0])

    ​		...

* jpackusdq dst, r1, r2

  * 编码

    funct4 = 0x5

  * 语义

    将r1和r2中的32位整数使用无符号饱和转换以16位无符号整数的形式保存在dst中

    和上面类似

* jpackusqdq dst, r1, r2

  * 编码

    funct4 = 0x6

  * 语义
  
    将r1和r2中的64位整数使用无符号饱和转换以32位无符号整数的形式保存在dst中
  
    和上面类似



### 算术运算
| opcode | funct4 | functype | r2    | r1    | dst   | zeroes   |
| ------ | ------ | -------- | ----- | ----- | ----- | -------- |
| 00001  | 4bits  | 4bits    | 3bits | 3bits | 3bits | 11bits 0 |

* jaddw dst, r1, r2

  * 编码

    functype = 0010

    funct4 = 0000

  * 语义

    for j = 0 to 15

    ​	i = j*16

    ​	dst[i+15:i] = r1[i+15:i] + r2[i+15:i]

    也就是16位整数的加法

* jaddd dst, r1, r2

  * 编码

    functype = 0010
    
    funct4 = 0001
    
  * 语义

    32位整数的加法

* jaddq dst, r1, r2

  * 编码

    functype = 0010
    
    funct4 = 0010
    
  * 语义

    64位整数的加法

* jsubw dst, r1, r2

  * 编码

    functype = 0011

    funct4 = 0000

  * 语义

    for j = 0 to 15

    ​	i = j*16

    ​	dst[i+15:i] = r1[i+15:i] - r2[i+15:i]

    也就是16位整数的减法

* jsubd dst, r1, r2

  * 编码

    functype = 0011
    
    funct4 = 0001
    
  * 语义

    32位整数的减法

* jsubq dst, r1, r2

  * 编码

    functype = 0011
    
    funct4 = 0010
    
  * 语义

    64位整数的减法

* jmulhw dst, r1, r2

  * 编码

    functype = 0100

    funct4 = 0000

  * 语义

    for j = 0 to 15

    ​	i = j*16

    ​	dst[i+15:i] = (r1[i+15:i] * r2[i+15:i])[31:16]

    也就是将r1，r2按照16位整数相乘，并将结果的高16位存放在dst

* jmulhd dst, r1, r2

  * 编码

    functype = 0100
    
    funct4 = 0001
    
  * 语义

    按照32位整数相乘并将结果的高32位存放在dst，类似上面

* jmullw dst, r1, r2

  * 编码

    functype = 0101
    
    funct4 = 0000
    
  * 语义

    按照16位整数相乘，存放结果的低16位

* jmullq dst, r1, r2

  * 编码

    functype = 0101
    
    funct4 = 0001
    
  * 语义

    按照32位整数相乘，存放结果的低32位

* jmulw dst, r1, r2

  * 编码

    functype = 0101

    funct4 = 0010

  * 语义

    将对应位置的32位整数饱和转换成16位整数后相乘，得到32位整数存放在对应位置

    即：r1=(0, a1, 0, a2, 0, a3, 0, a4......)  r2=(0, b1, 0, b2, 0, b3, 0, b4....) (数据为饱和转换后的16位)

    ​	dst = (a1 * b1, a2 * b2, a3 * b3, a4 * b4)

* jsrld dst, r1, r2

  * 编码

    functype = 0110
    
    funct4 = 0000
    
  * 语义

    将r1中的32位整数逻辑右移r2中存放数字的位数，结果放置在dst中

* jslldq dst, r1, r2

  * 编码

    functype = 0111
    
    funct4 = 0011
    
  * 语义

    将r1中的64位整数逻辑左移r2中存放数字的位数，结果放置在dst中

* jmaddw dst, r1, r2, imm

  * 编码

    functype = 1111

    funct4 = 0000

  * 语义

    将r1和r2中存放的16个16位整数分为四组

    第i组的四个对应位置的整数相乘，将四个乘积结果相加存放在dst[32 * i -1:(i-1) * 32]处
  
    e.g. r1(a1, a2, a3, a4,......)       r2(b1, b2, b3, b4....)
  
    ​		算出dst就为(a1 * b1 + a2 * b2 + a3 * b3 + a4 * b4, res2, res3, res4)
  
    是一个类似AVX指令集中dp_ps的操作
  
  

### 数据传输

* jmovdqa r1, m256

  * 编码
	| opcode | funct4 | functype | r1    | maddr  | zero |
	| ------ | ------ | -------- | ----- | ------ | ---- |
	| 00010  | 0011   | 0000     | 3bits | 16bits | 0    |
  

  * 语义

    将从m256地址开始的256位的整数数据读取到r1中

* jmovr r1, r2

  * 编码

    | opcode | funct4 | functype | r2    | r1    | dst  | zeroes   |
    | ------ | ------ | -------- | ----- | ----- | ---- | -------- |
    | 00001  | 0000   | 1000     | 3bits | 3bits | 000  | 11bits 0 |
  
  * 语义
  
    将r2的数据读取到r1中



### 数据类型转换

* jextractw r32, jmm, imm

  * 编码

    | opcode | funct3 | functype | addr   | jmm   | imm   |
    | ------ | ------ | -------- | ------ | ----- | ----- |
    | 00011  | 001    | 0000     | 16bits | 3bits | 3bits |
    
  * 语义
  
    将jmm中的第imm个32位整数存到r32指向的变量中



### 实现代码

有了上面的这些指令，我们就可以一次性计算四个像素点的值，因此将循环展开。恰好每四个像素点共用一套uv，方便我们实现。为了展示方便，这里直接将汇编指令和C++代码一起编写

需要用到的公式如下

~~~C
int R = (298 * (y - 16) + 411 * (v - 128) + 32) >> 8;
int B = (298 * (y - 16) + 519 * (u - 128) + 83) >> 8;
int G = (298 * (y - 16) - 101 * (u - 128) - 211 * (v - 128)  - 429) >> 8;
int newy = ((66 * R + 129 * G + 25 * B ) >> 8) + 16;
int newu = ((-38 * R - 74 * G + 112 * B) >> 8) + 128;
int newv = ((112 * R - 94 * G - 18 * B) >> 8) + 128;
~~~



~~~C
for (int alpha = 1; alpha < 255; alpha = alpha + 3) {
	    for (int i = 0; i < height; i ++)
	        for (int j = 0; j < width; j += 4) {
	            int index = i + j * width;
	            int uindex = width * height + (i >> 1) + (j >> 1) * (width >> 1);
	            int vindex = uindex + ((width * height) >> 2);
				short y = (uint8_t)yuv[index];
				short y1 = (uint8_t)yuv[index + 1];
                short y2 = (uint8_t)yuv[index + 2];
                short y3 = (uint8_t)yuv[index + 3];
				short u = (uint8_t)yuv[uindex];
				short v = (uint8_t)yuv[vindex];
                
                /* calculate RGB*/
                short Rpart[16] = {1, v, u, y,
                                  1, v, u, y1,
                                  1, v, u, y2,
                                  1, v, u, y3};
                short op[16] = {0, 128, 128, 16,
                           0, 128, 128, 16,
                           0, 128, 128, 16};
                jmovdqa jmm0, Rpart
                jmovdqa jmm1, op // move data to register
                jsubw jmm0, jmm0, jmm1 // 16bit sub, result stored in jmm0
                short Rop[16] = {32, 411, 0, 298,
                            32, 411, 0, 298,
                            32, 411, 0, 298,
                            32, 411, 0, 298};
                short Gop[16] = {-429, -211, -101, 298,
                            -429, -211, -101, 298,
                            -429, -211, -101, 298,
                            -429, -211, -101, 298};
                short Bop[16] = {83, 0, 519, 298,
                            83, 0, 519, 298,
                            83, 0, 519, 298,
                            83, 0, 519, 298}; // prepared oprands for calculating RGB
                jmovdqa jmm1, Rop
                jmaddw jmm2, jmm0, jmm1 // jmm2:(0, R1, 0, R2, 0, R3, 0, R4) 32bit-data
                jmovdqa jmm1, Gop
                jmaddw jmm3, jmm0, jmm1 // jmm3:(0, G1, 0, G2, 0, G3, 0, G4) 32bit-data
                jmovdqa jmm1, Bop
                jmaddw jmm4, jmm0, jmm1 // jmm4:(0, B1, 0, B2, 0, B3, 0, B4) 32bit-data
                short shift[16] = {0, 0, 0, 0,
                                  0, 0, 0, 0,
                                  0, 0, 0, 0,
                                  0, 0, 0, 32};
                jmovdqa jmm1, shift
                jslldq jmm3, jmm3, jmm1 // jmm3:(G1, 0, G2, 0, G3, 0, G4, 0) 32bit-data
                jaddd jmm2, jmm2, jmm3 // jmm2:(G1, R1, G2, R2, G3, R3, G4, R4) 32bit-data
                short eight[16] = {0, 0, 0, 0,
                                  0, 0, 0, 0,
                                  0, 0, 0, 0,
                                  0, 0, 0, 8};
                jmovdqa jmm1, eight
                jsrld jmm2, jmm2, jmm1 // jmm2:(0, G1 >> 8, 0, R1 >> 8...) 16bit-data
                    
                /*alpha mix*/
                short a[16] = {0, alpha, 0, alpha,
                              0, alpha, 0, alpha,
                              0, alpha, 0, alpha,
                              0, alpha, 0, alpha}
                jmovdqa jmm5, a
                jmulw jmm2, jmm2, jmm5 // jmm2:(alpha * (G1 >> 8), alpha * (R1 >> 8)...) 32bit-data
                jsrld jmm2, jmm2, jmm1 // 再次右移8位获得alpha混合后的R和G
                // jmm2:(0, aG1, 0, aR1, ...) 16-bit data
                jsrld jmm4, jmm4, jmm1 // jmm4:(0, 0, 0, B1 >> 8, 0, 0, 0, B2 >> 8...) 16-bit data
                jmulw jmm4, jmm4, jmm5 // jmm4:(0, alpha * (B1 >> 8)...) 32-bit data
                jsrld jmm4, jmm4, jmm1 // 再次右移8位获得alpha混合后的B
                // jmm4:(0, 0, 0, aB1, ....) 16-bit data
                short sixteen[16] = {0, 0, 0, 0,
                                    0, 0, 0, 0,
                                    0, 0, 0, 0,
                                    0, 0, 0, 16};
                jmovdqa jmm3, sixteen
                jslldq jmm4, jmm4, jmm3 // jmm4:(0, 0, aB1, 0, 0, 0, aB2, 0...) 16-bit data
                jaddw jmm4, jmm4, jmm2 // jmm4:(0, aG1, aB1, aR1, 0, aG2, aB2, aR2...) 16-bit data
                
                    
                /* calculate new yuv */
                short Yop[16] = {0, 129, 25, 66,
                            	0, 129, 25, 66,
                            	0, 129, 25, 66,
                            	0, 129, 25, 66};
                short Uop[16] = {0, -74, 112, -38,
                            	0, -74, 112, -38,
                            	0, -74, 112, -38,
                            	0, -73, 112, -38};
                short Vop[16] = {0, -94, -18, 112,
                            	0, -94, -18, 112,
                            	0, -94, -18, 112,
                            	0, -94, -18, 112};
                
                jmovdqa jmm0, Yop
                jmaddw jmm2, jmm0, jmm4
                jsrld jmm2, jmm2, jmm1 // shift right 8bits
                // jmm2:(0, 0, 0, res1, 0, 0, 0, res2...) 16-bits
                short yaddop[16] = {0, 0, 0, 16,
                               	   0, 0, 0, 16,
                                   0, 0, 0, 16,
                                   0, 0, 0, 16};
                
                jmovdqa jmm0, yaddop
                jaddw jmm2, jmm2, jmm0 // jmm2(0, 0, 0, newy1, ...) 16-bits
                
                //calculation for u and v is similar to y
                jmovdqa jmm0, Uop
                jmaddw jmm3, jmm0, jmm4
                jsrld jmm3, jmm3, jmm1
                short uvaddop[16] = {0, 0, 0, 128,
                               	   0, 0, 0, 128,
                                   0, 0, 0, 128,
                                   0, 0, 0, 128};
                jmovdqa jmm7, uvaddop
                jaddw jmm3, jmm3, jmm7 // jmm3(0, 0, 0, newu1, ...) 16-bits
                
                jmovdqa jmm0, Vop
                jmaddw jmm4, jmm0, jmm4
                jsrld jmm4, jmm4, jmm1
                jaddw jmm4, jmm4, jmm7 // jmm4(0, 0, 0, newv1, ...) 16-bits
                
                for (int pos = 0; pos < 4; pos++) {
                    int tmpy;
                    jextractw tmpy, jmm2, pos * 4
                    output[cnt][index + pos] = (char)tmpy;
                } // store newy
                
                int x;
                jextractw x, jmm3, 0
                output[cnt][uindex] = x;
                jextractw x, jmm4, 0
                output[cnt][vindex] = x; // store u and v
                
	        }
	        cnt++;
	    }
~~~

在这种情况下，我们每次计算4个位置的yuv，会进行6次加减运算，9次位移运算，2次乘法运算和6次我们定义的madd运算，根据avx中的dp_ps和乘法运算的延时关系（数据可以在intrinsics guide上看到），这样的madd运算大概是乘法运算的延时的2-3倍，而乘法运算大概是加减运算的延时的5倍，位移运算的延时和加减运算基本一致

因此如果设加减运算的延时为单位1，我们计算4个位置的yuv共用延时为：6 + 9 + 2 * 5 + 6 * 5 * 3 = 115（只考虑图像处理计算过程，不考虑数据传输、转化等过程，也不考虑index的计算），总指令数为23



而原本的整数计算代码

~~~C
for (int alpha = 1; alpha < 255; alpha = alpha + 3) {
	    for (int i = 0; i < width; i ++)
	        for (int j = 0; j < height; j ++) {
	            int index = i + j * width;
	            int uindex = width * height + (i >> 1) + (j >> 1) * (width >> 1);
	            int vindex = uindex + ((width * height) >> 2);
	            int y = (uint8_t)yuv[index];
	            int u = (uint8_t)yuv[uindex];
	            int v = (uint8_t)yuv[vindex];
	            int R = (298 * (y - 16) + 411 * (v - 128) + 32) >> 8;
	            R = (alpha * R) >> 8;
	            int B = (298 * (y - 16) + 519 * (u - 128) + 83) >> 8;
	            B = (alpha * B) >> 8;
	            int G = (298 * (y - 16) - 101 * (u - 128) - 211 * (v - 128)  - 429) >> 8;
	            G = (alpha * G) >> 8;
	            int newy = ((66 * R + 129 * G + 25 * B ) >> 8) + 16;
	        	int newu = ((-38 * R - 74 * G + 112 * B) >> 8) + 128;
	            int newv = ((112 * R - 94 * G - 18 * B) >> 8) + 128;
	            output[cnt][index] = (char)newy;
	            output[cnt][uindex] = (char)newu;
	            output[cnt][vindex] = (char)newv;
	        }
	        cnt++;
	    }
~~~

每次计算一个yuv，共用加减23次，乘法19次，位移9次，总指令数为51，总延时为：23 + 9 + 19 * 5 = 127

那么同样计算四个yuv，总指令数为204，比上面的方法多了181个，总延时为127 * 4 = 508， 是上面方法的4.42倍

通过定性分析，可以发现我们所构想的指令集可以大幅度的降低指令数并加速函数计算