# 4k.c

### 0.1
Hello world, thanks to zamfofex

32 bit: 171 bytes

### 0.2
Start of UCI

32 bit: 335 bytes

### 0.3
64 bit binary, thanks to zamfofex

32 bit: 335 bytes

64 bit: 414 bytes

### 0.4

Port 4ku base chess

32 bit: 4992 bytes

64 bit: 3872 bytes

### 0.5

All static functions, gcc 14.2

32- bit currently broken

64 bit: 2528 bytes

### 0.6

UCI compliant engine which prints the first move

64 bit: 3776 bytes

### 0.7

Add perft

64 bit: 3776 bytes


### 0.8

Perft NPS

64 bit: 3776 bytes

Perft: info depth 6 nodes 119060324 time 11034 nps 16843009

### 0.9

Remove hyperbola quintessence

64 bit: 3217 bytes

Perft: info depth 6 nodes 119060324 time 20995 nps 16843009


### 0.10

Fix legal movegen

64 bit: 3313 bytes

### 0.11

Material evaluation

64 bit: 3490 bytes

```py
Score of 4k.c-0.11 vs 4k.c-0.10: 858 - 0 - 9142  [0.543] 10000
...      4k.c-0.11 playing White: 404 - 0 - 4596  [0.540] 5000
...      4k.c-0.11 playing Black: 454 - 0 - 4546  [0.545] 5000
...      White vs Black: 404 - 454 - 9142  [0.497] 10000
Elo difference: 29.9 +/- 1.9, LOS: 100.0 %, DrawRatio: 91.4 %
```

### 0.12

Fix evaluation

64 bit: 3498 bytes

```py
Score of 4k.c-0.12 vs 4k.c-0.11: 181 - 43 - 9776  [0.507] 10000
...      4k.c-0.12 playing White: 91 - 18 - 4891  [0.507] 5000
...      4k.c-0.12 playing Black: 90 - 25 - 4885  [0.506] 5000
...      White vs Black: 116 - 108 - 9776  [0.500] 10000
Elo difference: 4.8 +/- 1.0, LOS: 100.0 %, DrawRatio: 97.8 %
```
