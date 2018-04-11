# Gaussian Elimination with various methods

1. One Process, One Thread :: [OneProcessOneThread.c](https://github.com/KeonHeeLee/Gaussian-elimination-with-thread/blob/master/src/OneProcessOneThread.c)
2. Multi Thread :: [MultiThread.c](https://github.com/KeonHeeLee/Gaussian-elimination-with-thread/blob/master/src/MultiThread.c)
3. Thread Pool :: [ThreadPool.c](https://github.com/KeonHeeLee/Gaussian-elimination-with-thread/blob/master/src/ThreadPool.c)
4. OpenMP Threading :: [ompThread.c](https://github.com/KeonHeeLee/Gaussian-elimination-with-thread/blob/master/src/ompThread.c)


## Performance result

<img src="https://github.com/KeonHeeLee/Gaussian-elimination-with-thread/blob/master/image/28.PNG">

<br>
- In the case of number 1, the longer the data grows, the longer it takes.<br>
- The number 2, 3, and 4 have different performance depending on the number of cores in the CPU.<br>
- If there are four cores, the execution time is reduced to near one quarter. After that, the execution time is increased.<br>
