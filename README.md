# CS537-Project3: Parallel Sort

CS537 Project3 codes by team Shihong Wang and Tong Xia

## Updates

### 2022.10.27: Project Detail Update!

Professor Remzi just release the Project instructions.

### 2022.10.29: Add test file generation shell

The discussion from piazza make it clear that what shold the test file be like, and how to sort each record. So, a shell generates the test file is created.

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
