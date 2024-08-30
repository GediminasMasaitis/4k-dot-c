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

### 0.13

Fixed-depth = 3 search

64 bit: 3513 bytes

```py
Score of 4k.c-0.13 vs 4k.c-0.12: 5722 - 4 - 4274  [0.786] 10000
...      4k.c-0.13 playing White: 2938 - 2 - 2060  [0.794] 5000
...      4k.c-0.13 playing Black: 2784 - 2 - 2214  [0.778] 5000
...      White vs Black: 2940 - 2786 - 4274  [0.508] 10000
Elo difference: 225.9 +/- 5.0, LOS: 100.0 %, DrawRatio: 42.7 %
```

### 0.14

Fix search

64 bit: 3513 bytes


### 0.15

Mate and stalemate dectection

64 bit: 3577 bytes

```py
Score of 4k.c-0.15 vs 4k.c-0.14: 554 - 196 - 4250  [0.536] 5000
...      4k.c-0.15 playing White: 280 - 94 - 2126  [0.537] 2500
...      4k.c-0.15 playing Black: 274 - 102 - 2124  [0.534] 2500
...      White vs Black: 382 - 368 - 4250  [0.501] 5000
Elo difference: 24.9 +/- 3.7, LOS: 100.0 %, DrawRatio: 85.0 %
```

### 0.16

Iterative deepening

64 bit: 3804 bytes

```py
info depth 1 score 0 time 0 pv a2a3
info depth 2 score 0 time 1 pv a2a3
info depth 3 score 0 time 3 pv a2a3
info depth 4 score 0 time 66 pv a2a3
info depth 5 score 100 time 1310 pv b2b3
info depth 6 score -100 time 31597 pv a2a3
```

```py
10+0.1:
Score of 4k.c-0.16 vs 4k.c-0.15: 318 - 88 - 594  [0.615] 1000
...      4k.c-0.16 playing White: 234 - 15 - 247  [0.721] 496
...      4k.c-0.16 playing Black: 84 - 73 - 347  [0.511] 504
...      White vs Black: 307 - 99 - 594  [0.604] 1000
Elo difference: 81.4 +/- 13.5, LOS: 100.0 %, DrawRatio: 59.4 %
```

### 0.17

Alpha-beta pruning

64 bit: 3856 bytes


```py
info depth 1 score 0 time 0 pv a2a3
info depth 2 score 0 time 0 pv a2a3
info depth 3 score 0 time 0 pv a2a3
info depth 4 score 0 time 1 pv a2a3
info depth 5 score 100 time 11 pv b2b3
info depth 6 score -100 time 231 pv a2a3
info depth 7 score 100 time 568 pv a2a3
info depth 8 score -100 time 9279 pv a2a3
info depth 9 score 100 time 36068 pv a2a3
```

```py
Score of 4k.c-0.17 vs 4k.c-0.16: 345 - 185 - 470  [0.580] 1000
...      4k.c-0.17 playing White: 298 - 8 - 194  [0.790] 500
...      4k.c-0.17 playing Black: 47 - 177 - 276  [0.370] 500
...      White vs Black: 475 - 55 - 470  [0.710] 1000
Elo difference: 56.1 +/- 15.7, LOS: 100.0 %, DrawRatio: 47.0 %
```

### 0.18

Centrality evaluation

64 bit: 3988 bytes

```py
info depth 1 score 40 time 0 pv b1c3
info depth 2 score 0 time 0 pv b1c3
info depth 3 score 40 time 3 pv b1c3
info depth 4 score 0 time 36 pv b1c3
info depth 5 score 62 time 269 pv e2e4
info depth 6 score -47 time 2285 pv b1c3
info depth 7 score 96 time 21554 pv e2e3
```

```py
Score of 4k.c-0.18 vs 4k.c-0.17: 377 - 217 - 406  [0.580] 1000
...      4k.c-0.18 playing White: 353 - 9 - 138  [0.844] 500
...      4k.c-0.18 playing Black: 24 - 208 - 268  [0.316] 500
...      White vs Black: 561 - 33 - 406  [0.764] 1000
Elo difference: 56.1 +/- 16.7, LOS: 100.0 %, DrawRatio: 40.6 %
```

### 0.19

Fail-hard approach

Implemented by No4b

64 bit: 3966 bytes

```py
info depth 1 score 40 time 0 pv b1c3
info depth 2 score 0 time 1 pv b1c3
info depth 3 score 40 time 3 pv b1c3
info depth 4 score 0 time 35 pv b1c3
info depth 5 score 62 time 264 pv e2e4
info depth 6 score -47 time 2220 pv b1c3
info depth 7 score 96 time 21311 pv e2e3
```

Test by No4b:
```
Passed simplification [-10; 0] on Pohl book (5+0.05)

Score of 4k_d vs 4k_m: 799 - 754 - 555 [0.511] 2108
... 4k_d playing White: 780 - 14 - 261 [0.863] 1055
... 4k_d playing Black: 19 - 740 - 294 [0.158] 1053
... White vs Black: 1520 - 33 - 555 [0.853] 2108
Elo difference: 7.4 +/- 12.7, LOS: 87.3 %, DrawRatio: 26.3 %
SPRT: llr 2.94 (100.0%), lbound -2.94, ubound 2.94 - H1 was accepted
```

### 0.20

Haswell arch build

64 bit: 3944 bytes

```py
info depth 1 score 40 time 0 pv b1c3
info depth 2 score 0 time 0 pv b1c3
info depth 3 score 40 time 2 pv b1c3
info depth 4 score 0 time 31 pv b1c3
info depth 5 score 62 time 232 pv e2e4
info depth 6 score -47 time 1942 pv b1c3
info depth 7 score 96 time 18390 pv e2e3
```

```py
Score of 4k.c-0.20 vs 4k.c-0.19: 484 - 467 - 549  [0.506] 1500
...      4k.c-0.20 playing White: 469 - 16 - 265  [0.802] 750
...      4k.c-0.20 playing Black: 15 - 451 - 284  [0.209] 750
...      White vs Black: 920 - 31 - 549  [0.796] 1500
Elo difference: 3.9 +/- 14.0, LOS: 70.9 %, DrawRatio: 36.6 %
```

### 0.21

Reduce eval constant size

32 bit: 5216 bytes
64 bit: 3904 bytes

```py
info depth 1 score 40 time 0 pv b1c3
info depth 2 score 0 time 0 pv b1c3
info depth 3 score 40 time 3 pv b1c3
info depth 4 score 0 time 31 pv b1c3
info depth 5 score 62 time 231 pv e2e4
info depth 6 score -47 time 1948 pv b1c3
info depth 7 score 96 time 18460 pv e2e3
```

```py
Score of 4k.c-0.21 vs 4k.c-0.20: 409 - 410 - 431  [0.500] 1250
...      4k.c-0.21 playing White: 393 - 21 - 211  [0.798] 625
...      4k.c-0.21 playing Black: 16 - 389 - 220  [0.202] 625
...      White vs Black: 782 - 37 - 431  [0.798] 1250
Elo difference: -0.3 +/- 15.6, LOS: 48.6 %, DrawRatio: 34.5 %
```

### 0.22

Split quantized PST evaluation

```py
info depth 1 score 56 time 0 pv b1c3
info depth 2 score 0 time 0 pv b1c3
info depth 3 score 52 time 2 pv b1c3
info depth 4 score 0 time 33 pv b1c3
info depth 5 score 73 time 293 pv b2b3
info depth 6 score -31 time 2281 pv b1c3
info depth 7 score 119 time 21953 pv e2e3
```

```py
Score of 4k.c-0.22 vs 4k.c-0.21: 698 - 570 - 732  [0.532] 2000
...      4k.c-0.22 playing White: 652 - 15 - 333  [0.819] 1000
...      4k.c-0.22 playing Black: 46 - 555 - 399  [0.245] 1000
...      White vs Black: 1207 - 61 - 732  [0.786] 2000
Elo difference: 22.3 +/- 12.1, LOS: 100.0 %, DrawRatio: 36.6 %
```

### 0.23

Size savings, thanks to kz04px

32 bit: 5104 bytes
64 bit: 3802 bytes

```py
perft 6
info depth 6 nodes 119060324 time 24256 nps 0
```

```py
info depth 1 score 56 time 0 pv b1c3
info depth 2 score 0 time 0 pv b1c3
info depth 3 score 52 time 2 pv b1c3
info depth 4 score 0 time 29 pv b1c3
info depth 5 score 73 time 255 pv b2b3
info depth 6 score -31 time 2003 pv b1c3
info depth 7 score 119 time 19306 pv e2e3
info depth 8 score -85 time 233349 pv b1c3
```

```py
Score of 4k.c-0.23 vs 4k.c-0.22: 334 - 322 - 345  [0.506] 1001
...      4k.c-0.23 playing White: 321 - 11 - 168  [0.810] 500
...      4k.c-0.23 playing Black: 13 - 311 - 177  [0.203] 501
...      White vs Black: 632 - 24 - 345  [0.804] 1001
Elo difference: 4.2 +/- 17.4, LOS: 68.0 %, DrawRatio: 34.5 %
```

### 0.24

Capture-first move ordering

32 bit: 5200 bytes
64 bit: 3898 bytes

```py
info depth 1 score 56 time 0 pv b1c3
info depth 2 score 0 time 0 pv b1c3
info depth 3 score 52 time 3 pv b1c3
info depth 4 score 0 time 35 pv b1c3
info depth 5 score 73 time 219 pv b2b3
info depth 6 score -31 time 1576 pv b1c3
info depth 7 score 119 time 9415 pv e2e3
info depth 8 score -85 time 101043 pv b1c3
```

```py
Score of 4k.c-0.24 vs 4k.c-0.23: 387 - 278 - 335  [0.554] 1000
...      4k.c-0.24 playing White: 352 - 2 - 147  [0.849] 501
...      4k.c-0.24 playing Black: 35 - 276 - 188  [0.259] 499
...      White vs Black: 628 - 37 - 335  [0.795] 1000
Elo difference: 38.0 +/- 17.6, LOS: 100.0 %, DrawRatio: 33.5 %

Score of 4k.c-0.24 vs 4ku-executable-strength3: 13 - 764 - 223  [0.124] 1000
...      4k.c-0.24 playing White: 13 - 318 - 167  [0.194] 498
...      4k.c-0.24 playing Black: 0 - 446 - 56  [0.056] 502
...      White vs Black: 459 - 318 - 223  [0.571] 1000
Elo difference: -338.8 +/- 22.9, LOS: 0.0 %, DrawRatio: 22.3 %

Score of 4k.c-0.24 vs tscp: 30 - 1367 - 103  [0.054] 1500
...      4k.c-0.24 playing White: 27 - 628 - 93  [0.098] 748
...      4k.c-0.24 playing Black: 3 - 739 - 10  [0.011] 752
...      White vs Black: 766 - 631 - 103  [0.545] 1500
Elo difference: -496.3 +/- 31.9, LOS: 0.0 %, DrawRatio: 6.9 %
```

### 0.25

Depth-limited qsearch

32 bit: 5334 bytes
64 bit: 3994 bytes

```py
info depth 1 score 56 time 0 nodes 20 pv b1c3
info depth 2 score 0 time 0 nodes 394 pv b1c3
info depth 3 score 52 time 3 nodes 6646 nps 2215333 pv b1c3
info depth 4 score 0 time 46 nodes 106855 nps 2322934 pv b1c3
info depth 5 score 20 time 696 nodes 1521281 nps 2185748 pv b1c3
info depth 6 score 0 time 7409 nodes 15623452 nps 2108712 pv b1c3
info depth 7 score 26 time 59571 nodes 118064690 nps 1981915 pv d2d3
```

```py
Score of 4k.c-0.25 vs 4k.c-0.24: 1221 - 475 - 1304  [0.624] 3000
...      4k.c-0.25 playing White: 1024 - 6 - 472  [0.839] 1502
...      4k.c-0.25 playing Black: 197 - 469 - 832  [0.409] 1498
...      White vs Black: 1493 - 203 - 1304  [0.715] 3000
Elo difference: 88.2 +/- 9.4, LOS: 100.0 %, DrawRatio: 43.5 %
```

### 0.26

Best-ply-move ordering

32 bit: 5376 bytes
64 bit: 4026 bytes

```py
info depth 1 score 56 time 0 nodes 20 pv b1c3
info depth 2 score 0 time 1 nodes 59 nps 59000 pv b1c3
info depth 3 score 52 time 1 nodes 523 nps 523000 pv b1c3
info depth 4 score 0 time 2 nodes 1692 nps 846000 pv b1c3
info depth 5 score 20 time 7 nodes 14412 nps 2058857 pv b1c3
info depth 6 score 0 time 43 nodes 57402 nps 1334930 pv b1c3
info depth 7 score 26 time 234 nodes 467210 nps 1996623 pv b1c3
info depth 8 score 0 time 1648 nodes 2450732 nps 1487094 pv b1c3
info depth 9 score 34 time 10016 nodes 19318696 nps 1928783 pv b1c3
info depth 10 score 0 time 73373 nodes 116816498 nps 1592091 pv b1c3
```

```py
Score of 4k.c-0.26 vs 4k.c-0.25: 654 - 417 - 429  [0.579] 1500
...      4k.c-0.26 playing White: 649 - 8 - 94  [0.927] 751
...      4k.c-0.26 playing Black: 5 - 409 - 335  [0.230] 749
...      White vs Black: 1058 - 13 - 429  [0.848] 1500
Elo difference: 55.4 +/- 15.0, LOS: 100.0 %, DrawRatio: 28.6 %
```

### 0.27

Treat moves as 64-bit integers

32 bit: 5408 bytes
64 bit: 3994 bytes

```py
info depth 1 score 56 time 0 nodes 20 pv b1c3
info depth 2 score 0 time 0 nodes 59 pv b1c3
info depth 3 score 52 time 1 nodes 523 nps 523000 pv b1c3
info depth 4 score 0 time 2 nodes 1692 nps 846000 pv b1c3
info depth 5 score 20 time 7 nodes 14412 nps 2058857 pv b1c3
info depth 6 score 0 time 42 nodes 57402 nps 1366714 pv b1c3
info depth 7 score 26 time 235 nodes 467210 nps 1988127 pv b1c3
info depth 8 score 0 time 1648 nodes 2450732 nps 1487094 pv b1c3
info depth 9 score 34 time 10160 nodes 19318696 nps 1901446 pv b1c3
info depth 10 score 0 time 74221 nodes 116816498 nps 1573900 pv b1c3
```

```
Score of 4k.c-0.27 vs 4ku-executable-stength3: 137 - 492 - 371  [0.323] 1000
...      4k.c-0.27 playing White: 132 - 114 - 251  [0.518] 497
...      4k.c-0.27 playing Black: 5 - 378 - 120  [0.129] 503
...      White vs Black: 510 - 119 - 371  [0.696] 1000
Elo difference: -129.0 +/- 17.5, LOS: 0.0 %, DrawRatio: 37.1 %
```

### 0.28

In-check extension

32 bit: 5408 bytes
64 bit: 3994 bytes

```py
info depth 1 score 56 time 0 nodes 20 pv b1c3
info depth 2 score 0 time 0 nodes 59 pv b1c3
info depth 3 score 52 time 1 nodes 523 nps 523000 pv b1c3
info depth 4 score 0 time 2 nodes 1696 nps 848000 pv b1c3
info depth 5 score 20 time 10 nodes 14426 nps 1442600 pv b1c3
info depth 6 score 0 time 55 nodes 60534 nps 1100618 pv b1c3
info depth 7 score 26 time 312 nodes 478381 nps 1533272 pv b1c3
info depth 8 score 0 time 2106 nodes 2675432 nps 1270385 pv b1c3
info depth 9 score 34 time 13891 nodes 21154897 nps 1522921 pv b1c3
info depth 10 score 0 time 111026 nodes 155980657 nps 1404902 pv b1c3
```

```py
Score of 4k.c-0.28 vs 4k.c-0.27: 1309 - 1173 - 518  [0.523] 3000
...      4k.c-0.28 playing White: 1281 - 6 - 214  [0.925] 1501
...      4k.c-0.28 playing Black: 28 - 1167 - 304  [0.120] 1499
...      White vs Black: 2448 - 34 - 518  [0.902] 3000
Elo difference: 15.8 +/- 11.3, LOS: 99.7 %, DrawRatio: 17.3 %
```

### 0.29

Tempo evaluation

32 bit: 5408 bytes
64 bit: 3994 bytes

```
info depth 1 score 40 time 0 nodes 20 pv b1c3
info depth 2 score 16 time 0 nodes 59 pv b1c3
info depth 3 score 36 time 1 nodes 523 nps 523000 pv b1c3
info depth 4 score 16 time 2 nodes 1687 nps 843500 pv b1c3
info depth 5 score 4 time 10 nodes 14422 nps 1442200 pv b1c3
info depth 6 score 14 time 54 nodes 62783 nps 1162648 pv b1c3
info depth 7 score 10 time 310 nodes 473798 nps 1528380 pv b1c3
info depth 8 score 16 time 1997 nodes 2495781 nps 1249765 pv b1c3
info depth 9 score 16 time 19138 nodes 29979864 nps 1566509 pv b1c3
info depth 10 score 16 time 120909 nodes 166044509 nps 1373301 pv b1c3
```

```py
Score of 4k.c-0.29 vs 4k.c-0.28: 28795 - 27977 - 12228  [0.506] 69000
...      4k.c-0.29 playing White: 28561 - 197 - 5743  [0.911] 34501
...      4k.c-0.29 playing Black: 234 - 27780 - 6485  [0.101] 34499
...      White vs Black: 56341 - 431 - 12228  [0.905] 69000
Elo difference: 4.1 +/- 2.3, LOS: 100.0 %, DrawRatio: 17.7 %

Score of 4k.c-0.29 vs 4ku-executable-stength3: 174 - 436 - 390  [0.369] 1000
...      4k.c-0.29 playing White: 168 - 64 - 267  [0.604] 499
...      4k.c-0.29 playing Black: 6 - 372 - 123  [0.135] 501
...      White vs Black: 540 - 70 - 390  [0.735] 1000
Elo difference: -93.2 +/- 17.0, LOS: 0.0 %, DrawRatio: 39.0 %

Score of 4k.c-0.29 vs tscp1.82: 196 - 587 - 217  [0.304] 1000
...      4k.c-0.29 playing White: 186 - 135 - 177  [0.551] 498
...      4k.c-0.29 playing Black: 10 - 452 - 40  [0.060] 502
...      White vs Black: 638 - 145 - 217  [0.747] 1000
Elo difference: -143.5 +/- 20.2, LOS: 0.0 %, DrawRatio: 21.7 %
```
