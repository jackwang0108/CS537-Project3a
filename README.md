# CS537-Project3: Parallel Sort

CS537 Project3 codes by team Shihong Wang and Tong Xia

## Updates

### 2022.10.27: Project Detail Update!

Professor Remzi just release the Project instructions.

### 2022.10.29: Add test file generation shell

The discussion from piazza make it clear that what shold the test file be like, and how to sort each record. So, a shell generates the test file is created.

### 2022.10.4: Finished Parallel Sort Frame

多线程debug很他妈痛苦，一个bug修了一天

## How to develop

### tools

There is a few shell tools for develop: `init.sh`, `autobuild.sh` and `gentest.sh`

To use them, first run
```shell
cd path-to-project
source init.sh
```
and then, you can use the shell tools with tab-completion

`autobuild.sh` is used for automatically build the project.
```shell
autobu<Tab><Enter>
```

and `gentest.sh` is used for aotumatically generate test file.
```shell
gent<Tab><Enter>
```


## Things need to know

### 1. About key and value

> **clarification on input**
>
>> The input file will consist of records; within each record is a
>> key. The key is the first four bytes of the record. The records are
>> fixed-size, and are each 100 bytes (which includes the key).
>>
>
> **the instructors' answer**
>
> You just sort by they key. The rest of the record is the value which does not affect the sort. See also the answer in [@1200](1200)

source: https://piazza.com/class/l7kt5onxp5h4hp/post/1181

### 2. About input files

> **Can we get a sample input file for p3a?**
>
>> Can we get a sample input file for the below specification? Does it mean that each line of the input file is a record which has 1 key and 24 integers to be sorted?
>>
>
> **the students' answer**
>
> My understanding from the paper and the limited input from the course staff is that the input file is simply a block of bytes. If you have a 1000 byte file, then it will have 10 records of 100 bytes. The first 4 bytes of each of these records is the key. You sort by the key, making sure to keep track of which key maps to which record. You then write this new ordering out again as block of bytes.
>
> If my understanding is correct, then an easy way to generate test files is:
>
> ```shell
> dd if=/dev/random of=output.bin bs=1000 count=1 iflag=fullblock
> ```
>
> **the instructors' answer**
>
> There aren't any delimiters (newlines or commas) in the files, you just split the file into 100 byte pieces. That should make it pretty easy to parse.
>
> You will then just convert/cast the first 4 bytes into an integer and sort all record by that.
>
> We will definitely release some sample input soon, probably with the tests.

source: https://piazza.com/class/l7kt5onxp5h4hp/post/1200



### 3. About test file generator

> Here is the testcase generator we will be using to generate the official testcases, and you all can use for your own debugging and such. You will not be able to read this test file due to the fact it is in binary format. If you feel you need something to visualize, you can just type a series of (100 x n) characters to create a test file with n entries.
>
> ```c
> #include <fcntl.h>
> #include <assert.h>
> #include <stdio.h>
> #include <stdlib.h>
> #include <unistd.h>
> 
> typedef struct rec {
>     int key;
>     int value[24]; // 96 bytes
> } rec_t;
> 
> int main(int argc, char *argv[]) {
>     if (argc != 3) {
>         fprintf(stderr, "generate numkeys file\n");
>         exit(1);
>     }
>     int n = atoi(argv[1]);
>     char *file = argv[2];
>     int fd = open(file, O_CREAT | O_WRONLY | O_TRUNC, 0666);
>     if (fd < 0) {
>         perror("open");
>         exit(1);
>     }
>     int k;
>     for (k = 0; k < n; k++) {
>         long u = random();
>         rec_t r;
>         r.key = u;
>         int i;
>         for (i = 0; i < 24; i++) {
>             r.value[i] = random();
>         }
>         int rc = write(fd, &r, sizeof(rec_t));
>         assert(rc == sizeof(rec_t));
>     }
>     close(fd);
>     return 0;
> }
> ```

source: https://piazza.com/class/l7kt5onxp5h4hp/post/1283



### 4. Project 3A FAQ

https://piazza.com/class/l7kt5onxp5h4hp/post/1283



### 5. Little-endian Byte Ordering

> **Hexdump and int read not same! p3a**
>
> Hi, I created a binary file from code in a piazza post of size 4 bytes. I just wanted to see if i get an int from these bytes. I used the hexdump linux function and got this:
>
> ![image.png](https://piazza.com/redirect/s3?bucket=uploads&prefix=paste%2Fkeij2x759i79y%2F4a9dc5766721994041051c7a62c2799b1a4761133e1e05f336ab95e3cc6b2930%2Fimage.png)
>
> which is
>
> ![image.png](https://piazza.com/redirect/s3?bucket=uploads&prefix=paste%2Fkeij2x759i79y%2F11819789cf77118b0bbffd024b7248af6b1d7de91a496baf8c8efceac586ee98%2Fimage.png)
>
>  in integer. Now, I used my code to read in the same file but read it as an int:
>
> ![image.png](https://piazza.com/redirect/s3?bucket=uploads&prefix=paste%2Fkeij2x759i79y%2Ffae450a5aa35d85f9590fee997b56ba481204b4cb8915b99a1cd3d721d56965c%2Fimage.png)
>
> For reference, fread (pointer to store info, size of each item, number of items, file pointer) is the syntax.
>
> The output I got was:
>
> ![image.png](https://piazza.com/redirect/s3?bucket=uploads&prefix=paste%2Fkeij2x759i79y%2Fb6e8a19d035538ba75146f4b7e76222b82da7dcced18b854e5aaa3da1dc56a87%2Fimage.png)
>
> Why are the two ints not the same?
>
> P.S. Someone please create a folder on Piazza for p3a so I can appropriately add the flair.
>
> **the instructors' answer**
>
> If you convert 1826852556 to hex, you will get 6CE38ECC.
>
> Does 6CE38ECC look familiar? 
>
> x86 processors use little-endian byte ordering, so what you see is different than what the machine sees
>
> I have created a folder for p3a, thanks for reminding 

source: https://piazza.com/class/l7kt5onxp5h4hp/post/1248

## Project Instuctions (from website)

### Important Dates and Other Stuff

**Due:** Monday, 11/07 by midnight

**This project is to be done in a group of size one or two.**

**Tests**:Tests will be made available shortly.

### Questions?

Send questions using piazza or use office hours. If the question is about your code, copy all of of your code into your handin directory (details below) and include your login in your email (you are free to modify the contents of your handin directory prior to the due date). Do **not** put code snippets into piazza (unless they are very short). Also include all other relevant information, such as cutting and pasting what you typed and the results from the screen. In general, the more information you give, the more we can help.

### Overview

The basic project description is found `<a href=https://github.com/remzi-arpacidusseau/ostep-projects/tree/master/concurrency-sort>`here.`</a>`  Please read this carefully in order to understand exactly what to do.

This project is to be done on the lab machines (listed `<a href=https://csl.cs.wisc.edu/docs/csl/2012-08-16-instructional-facilities/>`
here `</a>` ), so you can learn more about programming in C on a typical UNIX-based platform (Linux)

### Differences

There are no differences from the github specified project.

### Notes

`<b>`Before beginning:`</b>` If you don't remember much about the Unix/C environment, read `<a href=http://pages.cs.wisc.edu/~remzi/OSTEP/lab-tutorial.pdf>`this tutorial.`</a>`  It has some useful tips for programming.

`<b>`This project should be done in groups of one or two.`</b>` Copying code (from other groups) is considered cheating. Read `<a href=../dontcheat.html>`this `</a>`  for more info on what is OK and what is not. Please help us all have a good semester by not doing this.

### Contest

There will be a contest for the **fastest** sort. More details coming soon. Winner gets a choice of OSTEP merchandise!

### Handing It In

You should turn in two files, `<code>`psort.c `</code>` and `<code>`Makefile `</code>` . The makefile should build the `<code>`psort `</code>` executable.

The handin directory is `<code>`~cs537-1/handin/login/p3a `</code>` where `<code>`login `</code>` is your login. For example, Remzi's login is `<code>`remzi `</code>` , and thus he would copy his beautiful code into `<code>`~cs537-1/handin/remzi/p3a `</code>` . Copying of these files is accomplished with the `<code>`cp `</code>` program, as follows:

```shell
 cp psort.c Makefile ~cs537-1/handin/remzi/p3a/
```

When done, type `<code>`ls ~cs537-1/handin/remzi/p3a `</code>` to see that all the files are in place correctly.

```shell
ls ~cs537-1/handin/remzi/p3a
```

Finally, in your p3a directory, please include a `README` file. In there, describe what you did a little bit. There is no particular requirement for the length of the `README`; just get in the habit of writing a little bit about what you did, so that another human could understand it.

We will add directions for how to handin something when you have a group of two shortly.
