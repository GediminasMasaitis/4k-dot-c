# 4k.c

### 0.1
Hello world, thanks to zamfofex

32 bit: 171 bytes

### 0.2
Start of UCI

32 bit: 335 bytes (+164)

### 0.3
64 bit binary, thanks to zamfofex

32 bit: 335 bytes (=)
64 bit: 414 bytes

### 0.4

Port 4ku base chess

32 bit: 4992 bytes (+4657)
64 bit: 3872 bytes (+3458)

### 0.5

All static functions, gcc 14.2

32- bit currently broken
64 bit: 2528 bytes (-1344)

### 0.6

UCI compliant engine which prints the first move

64 bit: 3776 bytes (+1248)

### 0.7

Add perft

64 bit: 3776 bytes (=)


### 0.8

Perft NPS

64 bit: 3776 bytes (=)

Perft: info depth 6 nodes 119060324 time 11034 nps 16843009

### 0.9

Remove hyperbola quintessence

64 bit: 3217 bytes (-559)

Perft: info depth 6 nodes 119060324 time 20995 nps 16843009


### 0.10

Fix legal movegen

64 bit: 3313 bytes (+96)

### 0.11

Material evaluation

64 bit: 3490 bytes (+117)

```py
Score of 4k.c-0.11 vs 4k.c-0.10: 858 - 0 - 9142  [0.543] 10000
...      4k.c-0.11 playing White: 404 - 0 - 4596  [0.540] 5000
...      4k.c-0.11 playing Black: 454 - 0 - 4546  [0.545] 5000
...      White vs Black: 404 - 454 - 9142  [0.497] 10000
Elo difference: 29.9 +/- 1.9, LOS: 100.0 %, DrawRatio: 91.4 %
```

### 0.12

Fix evaluation

64 bit: 3498 bytes (-8)

```py
Score of 4k.c-0.12 vs 4k.c-0.11: 181 - 43 - 9776  [0.507] 10000
...      4k.c-0.12 playing White: 91 - 18 - 4891  [0.507] 5000
...      4k.c-0.12 playing Black: 90 - 25 - 4885  [0.506] 5000
...      White vs Black: 116 - 108 - 9776  [0.500] 10000
Elo difference: 4.8 +/- 1.0, LOS: 100.0 %, DrawRatio: 97.8 %
```

### 0.13

Fixed-depth = 3 search

64 bit: 3513 bytes (+15)

```py
Score of 4k.c-0.13 vs 4k.c-0.12: 5722 - 4 - 4274  [0.786] 10000
...      4k.c-0.13 playing White: 2938 - 2 - 2060  [0.794] 5000
...      4k.c-0.13 playing Black: 2784 - 2 - 2214  [0.778] 5000
...      White vs Black: 2940 - 2786 - 4274  [0.508] 10000
Elo difference: 225.9 +/- 5.0, LOS: 100.0 %, DrawRatio: 42.7 %
```

### 0.14

Fix search

64 bit: 3513 bytes (=)


### 0.15

Mate and stalemate dectection

64 bit: 3577 bytes (+64)

```py
Score of 4k.c-0.15 vs 4k.c-0.14: 554 - 196 - 4250  [0.536] 5000
...      4k.c-0.15 playing White: 280 - 94 - 2126  [0.537] 2500
...      4k.c-0.15 playing Black: 274 - 102 - 2124  [0.534] 2500
...      White vs Black: 382 - 368 - 4250  [0.501] 5000
Elo difference: 24.9 +/- 3.7, LOS: 100.0 %, DrawRatio: 85.0 %
```

### 0.16

Iterative deepening

64 bit: 3804 bytes (+227)

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

64 bit: 3856 bytes (+52)


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

64 bit: 3988 bytes (+132)

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

64 bit: 3966 bytes (-22)

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

64 bit: 3944 bytes (-22)

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
64 bit: 3904 bytes (-40)

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

32 bit: 5264 bytes (+48)
64 bit: 3967 bytes (+63)

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

32 bit: 5104 bytes (-160)
64 bit: 3802 bytes (-165)

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

32 bit: 5200 bytes (+96)
64 bit: 3898 bytes (+92)

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

32 bit: 5334 bytes (+134)
64 bit: 3994 bytes (+96)

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

32 bit: 5376 bytes (+42)
64 bit: 4026 bytes (+32)

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

32 bit: 5408 bytes (+32)
64 bit: 3994 bytes (-32)

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

```py
Score of 4k.c-0.27 vs 4ku-executable-stength3: 137 - 492 - 371  [0.323] 1000
...      4k.c-0.27 playing White: 132 - 114 - 251  [0.518] 497
...      4k.c-0.27 playing Black: 5 - 378 - 120  [0.129] 503
...      White vs Black: 510 - 119 - 371  [0.696] 1000
Elo difference: -129.0 +/- 17.5, LOS: 0.0 %, DrawRatio: 37.1 %
```

### 0.28

In-check extension

32 bit: 5408 bytes (=)
64 bit: 3994 bytes (=)

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

32 bit: 5408 bytes (=)
64 bit: 3994 bytes (=)

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

### 0.30

Reduce bytes

32 bit: 5408 bytes (=)
64 bit: 3962 bytes (-32)

```py
perft 6
info depth 6 nodes 119060324 time 25042 nps 4754425
```

### 0.31

Optimize ray function

32 bit: 5376 bytes (-32)
64 bit: 3898 bytes (-36)

```py
perft 6
info depth 6 nodes 119060324 time 15036 nps 7918350
```

```py
info depth 1 score 40 time 0 nodes 20 pv b1c3
info depth 2 score 16 time 0 nodes 59 pv b1c3
info depth 3 score 36 time 1 nodes 523 nps 523000 pv b1c3
info depth 4 score 16 time 2 nodes 1687 nps 843500 pv b1c3
info depth 5 score 4 time 7 nodes 14422 nps 2060285 pv b1c3
info depth 6 score 14 time 34 nodes 62783 nps 1846558 pv b1c3
info depth 7 score 10 time 201 nodes 473798 nps 2357203 pv b1c3
info depth 8 score 16 time 1230 nodes 2495781 nps 2029090 pv b1c3
info depth 9 score 16 time 12198 nodes 29979864 nps 2457768 pv b1c3
info depth 10 score 16 time 75922 nodes 166044509 nps 2187040 pv b1c3
```

```py
Score of 4k.c-0.30 vs 4k.c-0.29: 1271 - 1188 - 541  [0.514] 3000
...      4k.c-0.30 playing White: 1259 - 5 - 237  [0.918] 1501
...      4k.c-0.30 playing Black: 12 - 1183 - 304  [0.109] 1499
...      White vs Black: 2442 - 17 - 541  [0.904] 3000
Elo difference: 9.6 +/- 11.2, LOS: 95.3 %, DrawRatio: 18.0 %
```

### 0.32

Reduce bytes

32 bit: 5328 bytes (-48)
64 bit: 3858 bytes (-40)

```py
info depth 1 score 40 time 0 nodes 20 pv b1c3
info depth 2 score 16 time 0 nodes 59 pv b1c3
info depth 3 score 36 time 0 nodes 523 pv b1c3
info depth 4 score 16 time 1 nodes 1687 nps 1687000 pv b1c3
info depth 5 score 4 time 6 nodes 14422 nps 2403666 pv b1c3
info depth 6 score 14 time 33 nodes 62783 nps 1902515 pv b1c3
info depth 7 score 10 time 200 nodes 473798 nps 2368990 pv b1c3
info depth 8 score 16 time 1246 nodes 2495781 nps 2003034 pv b1c3
info depth 9 score 16 time 12323 nodes 29979864 nps 2432838 pv b1c3
info depth 10 score 16 time 77434 nodes 166044509 nps 2144335 pv b1c3
```

### 0.33

Fix UCI time parsing bug

32 bit: 5312 bytes (-12)
64 bit: 3842 bytes (-16)

```py
info depth 1 score 40 time 0 nodes 20 pv b1c3
info depth 2 score 16 time 0 nodes 59 pv b1c3
info depth 3 score 36 time 1 nodes 523 nps 523000 pv b1c3
info depth 4 score 16 time 1 nodes 1687 nps 1687000 pv b1c3
info depth 5 score 4 time 8 nodes 14422 nps 1802750 pv b1c3
info depth 6 score 14 time 35 nodes 62783 nps 1793800 pv b1c3
info depth 7 score 10 time 204 nodes 473798 nps 2322539 pv b1c3
info depth 8 score 16 time 1254 nodes 2495781 nps 1990255 pv b1c3
info depth 9 score 16 time 12671 nodes 29979864 nps 2366021 pv b1c3
info depth 10 score 16 time 78500 nodes 166044509 nps 2115216 pv b1c3
```

```py
Score of 4k.c-0.33 vs 4k.c-0.32: 528 - 105 - 367  [0.712] 1000
...      4k.c-0.33 playing White: 404 - 1 - 98  [0.901] 503
...      4k.c-0.33 playing Black: 124 - 104 - 269  [0.520] 497
...      White vs Black: 508 - 125 - 367  [0.692] 1000
Elo difference: 156.8 +/- 17.7, LOS: 100.0 %, DrawRatio: 36.7 %

Score of 4k.c-0.33 vs 4ku-executable-stength3: 373 - 127 - 500  [0.623] 1000
...      4k.c-0.33 playing White: 219 - 49 - 234  [0.669] 502
...      4k.c-0.33 playing Black: 154 - 78 - 266  [0.576] 498
...      White vs Black: 297 - 203 - 500  [0.547] 1000
Elo difference: 87.3 +/- 15.2, LOS: 100.0 %, DrawRatio: 50.0 %

Score of 4k.c-0.33 vs tscp1.82: 393 - 250 - 357  [0.572] 1000
...      4k.c-0.33 playing White: 212 - 94 - 194  [0.618] 500
...      4k.c-0.33 playing Black: 181 - 156 - 163  [0.525] 500
...      White vs Black: 368 - 275 - 357  [0.546] 1000
Elo difference: 50.0 +/- 17.3, LOS: 100.0 %, DrawRatio: 35.7 %
```

### 0.34

MVV move ordering

Implemented by No4b

32 bit: 5312 bytes (=)
64 bit: 3844 bytes (+2)

```py
info depth 1 score 40 time 0 nodes 20 pv b1c3
info depth 2 score 16 time 0 nodes 59 pv b1c3
info depth 3 score 36 time 0 nodes 523 pv b1c3
info depth 4 score 16 time 1 nodes 1665 nps 1665000 pv b1c3
info depth 5 score 4 time 6 nodes 14320 nps 2386666 pv b1c3
info depth 6 score 14 time 33 nodes 59486 nps 1802606 pv b1c3
info depth 7 score 10 time 197 nodes 461490 nps 2342588 pv b1c3
info depth 8 score 16 time 1164 nodes 2238356 nps 1922986 pv b1c3
info depth 9 score 16 time 11806 nodes 28230494 nps 2391198 pv b1c3
info depth 10 score 16 time 70831 nodes 147757798 nps 2086061 pv b1c3
```

Test performed by No4b: 
```py
Score of 4k_d vs 4k_m: 635 - 507 - 1258 [0.527] 2400
... 4k_d playing White: 369 - 219 - 612 [0.563] 1200
... 4k_d playing Black: 266 - 288 - 646 [0.491] 1200
... White vs Black: 657 - 485 - 1258 [0.536] 2400
Elo difference: 18.5 +/- 9.6, LOS: 100.0 %, DrawRatio: 52.4 %
```

```py
Score of 4k.c-0.34 vs 4k.c-0.33: 407 - 276 - 817  [0.544] 1500
...      4k.c-0.34 playing White: 205 - 125 - 421  [0.553] 751
...      4k.c-0.34 playing Black: 202 - 151 - 396  [0.534] 749
...      White vs Black: 356 - 327 - 817  [0.510] 1500
Elo difference: 30.4 +/- 11.8, LOS: 100.0 %, DrawRatio: 54.5 %
```

### 0.35

Reverse futility pruning

32 bit: 5328 bytes (+12)
64 bit: 3856 bytes (+12)

```py
info depth 1 score 40 time 0 nodes 20 pv b1c3
info depth 2 score 16 time 0 nodes 59 pv b1c3
info depth 3 score 36 time 0 nodes 523 pv b1c3
info depth 4 score 16 time 1 nodes 1665 nps 1665000 pv b1c3
info depth 5 score 4 time 6 nodes 14326 nps 2387666 pv b1c3
info depth 6 score 14 time 27 nodes 49185 nps 1821666 pv b1c3
info depth 7 score 10 time 184 nodes 444454 nps 2415510 pv b1c3
info depth 8 score 16 time 1003 nodes 1974087 nps 1968182 pv b1c3
info depth 9 score 16 time 10542 nodes 25976472 nps 2464093 pv b1c3
info depth 10 score 16 time 61508 nodes 128283178 nps 2085634 pv b1c3
```

Test performed by No4b: 
```py
Score of 4k_d vs 4k_m: 225 - 167 - 487  [0.533] 879
...      4k_d playing White: 143 - 63 - 232  [0.591] 438
...      4k_d playing Black: 82 - 104 - 255  [0.475] 441
...      White vs Black: 247 - 145 - 487  [0.558] 879
Elo difference: 23.0 +/- 15.3, LOS: 99.8 %, DrawRatio: 55.4 %
SPRT: llr 2.95 (100.1%), lbound -2.94, ubound 2.94 - H1 was accepted
```

```py
Score of 4k.c-0.35 vs 4k.c-0.34: 262 - 192 - 546  [0.535] 1000
...      4k.c-0.35 playing White: 123 - 97 - 281  [0.526] 501
...      4k.c-0.35 playing Black: 139 - 95 - 265  [0.544] 499
...      White vs Black: 218 - 236 - 546  [0.491] 1000
Elo difference: 24.4 +/- 14.5, LOS: 99.9 %, DrawRatio: 54.6 %

Score of 4k.c-0.35 vs tscp1.82: 429 - 197 - 374  [0.616] 1000
...      4k.c-0.35 playing White: 225 - 75 - 201  [0.650] 501
...      4k.c-0.35 playing Black: 204 - 122 - 173  [0.582] 499
...      White vs Black: 347 - 279 - 374  [0.534] 1000
Elo difference: 82.1 +/- 17.2, LOS: 100.0 %, DrawRatio: 37.4 %
```

### 0.36

Fix node counting

32 bit: 5328 bytes (=)
64 bit: 3856 bytes (=)

```py
info depth 1 score 40 time 0 nodes 20 pv b1c3
info depth 2 score 16 time 0 nodes 79 pv b1c3
info depth 3 score 36 time 0 nodes 602 pv b1c3
info depth 4 score 16 time 1 nodes 2267 nps 2267000 pv b1c3
info depth 5 score 4 time 6 nodes 16593 nps 2765500 pv b1c3
info depth 6 score 14 time 26 nodes 65778 nps 2529923 pv b1c3
info depth 7 score 10 time 184 nodes 510232 nps 2773000 pv b1c3
info depth 8 score 16 time 1012 nodes 2484319 nps 2454860 pv b1c3
info depth 9 score 16 time 10557 nodes 28460791 nps 2695916 pv b1c3
info depth 10 score 16 time 60433 nodes 156743969 nps 2593681 pv b1c3
```

### 0.37

Full time management

32 bit: 5376 bytes (+48)
64 bit: 3952 bytes (+96)

```py
Finished game 207 (4k.c-0.37 vs 4k.c-0.36): 1-0 {Black's connection stalls}
Score of 4k.c-0.37 vs 4k.c-0.36: 54 - 62 - 100  [0.481] 216
Finished game 223 (4k.c-0.37 vs 4k.c-0.36): * {No result}
Score of 4k.c-0.37 vs 4k.c-0.36: 54 - 62 - 100  [0.481] 216
Finished game 222 (4k.c-0.36 vs 4k.c-0.37): * {No result}
Score of 4k.c-0.37 vs 4k.c-0.36: 54 - 62 - 100  [0.481] 216
Finished game 220 (4k.c-0.36 vs 4k.c-0.37): * {No result}
Score of 4k.c-0.37 vs 4k.c-0.36: 54 - 62 - 100  [0.481] 216
Finished game 218 (4k.c-0.36 vs 4k.c-0.37): * {No result}
Score of 4k.c-0.37 vs 4k.c-0.36: 54 - 62 - 100  [0.481] 216
...      4k.c-0.37 playing White: 38 - 20 - 51  [0.583] 109
...      4k.c-0.37 playing Black: 16 - 42 - 49  [0.379] 107
...      White vs Black: 80 - 36 - 100  [0.602] 216
Elo difference: -12.9 +/- 34.0, LOS: 22.9 %, DrawRatio: 46.3 %
Finished game 216 (4k.c-0.36 vs 4k.c-0.37): * {No result}
Score of 4k.c-0.37 vs 4k.c-0.36: 54 - 62 - 100  [0.481] 216
Finished game 227 (4k.c-0.37 vs 4k.c-0.36): * {No result}
Score of 4k.c-0.37 vs 4k.c-0.36: 54 - 62 - 100  [0.481] 216
Finished game 224 (4k.c-0.36 vs 4k.c-0.37): * {No result}
Score of 4k.c-0.37 vs 4k.c-0.36: 54 - 62 - 100  [0.481] 216
Finished game 219 (4k.c-0.37 vs 4k.c-0.36): * {No result}
Score of 4k.c-0.37 vs 4k.c-0.36: 54 - 62 - 100  [0.481] 216
Finished game 225 (4k.c-0.37 vs 4k.c-0.36): * {No result}
Score of 4k.c-0.37 vs 4k.c-0.36: 54 - 62 - 100  [0.481] 216
Finished game 221 (4k.c-0.37 vs 4k.c-0.36): * {No result}
Score of 4k.c-0.37 vs 4k.c-0.36: 54 - 62 - 100  [0.481] 216
Finished game 226 (4k.c-0.36 vs 4k.c-0.37): * {No result}
Score of 4k.c-0.37 vs 4k.c-0.36: 54 - 62 - 100  [0.481] 216
...      4k.c-0.37 playing White: 38 - 20 - 51  [0.583] 109
...      4k.c-0.37 playing Black: 16 - 42 - 49  [0.379] 107
...      White vs Black: 80 - 36 - 100  [0.602] 216
Elo difference: -12.9 +/- 34.0, LOS: 22.9 %, DrawRatio: 46.3 %
Finished match
```

```py
Score of 4k.c-0.37 vs stash11: 675 - 641 - 684  [0.508] 2000
...      4k.c-0.37 playing White: 387 - 255 - 358  [0.566] 1000
...      4k.c-0.37 playing Black: 288 - 386 - 326  [0.451] 1000
...      White vs Black: 773 - 543 - 684  [0.557] 2000
Elo difference: 5.9 +/- 12.3, LOS: 82.6 %, DrawRatio: 34.2 %
```

### 0.38

Smaller TM using globals

32 bit: 5376 bytes (-24)
64 bit: 3944 bytes (-8)

```py
info depth 1 score 40 time 0 nodes 20 pv b1c3
info depth 2 score 16 time 0 nodes 79 pv b1c3
info depth 3 score 36 time 0 nodes 602 pv b1c3
info depth 4 score 16 time 1 nodes 2267 nps 2267000 pv b1c3
info depth 5 score 4 time 5 nodes 16593 nps 3318600 pv b1c3
info depth 6 score 14 time 24 nodes 65778 nps 2740750 pv b1c3
info depth 7 score 10 time 169 nodes 510232 nps 3019124 pv b1c3
info depth 8 score 16 time 927 nodes 2484319 nps 2679955 pv b1c3
info depth 9 score 16 time 9739 nodes 28460791 nps 2922352 pv b1c3
info depth 10 score 16 time 56980 nodes 156743969 nps 2750859 pv b1c3
```

### 0.39

Principal variation search

32 bit: 5432 bytes (+56)
64 bit: 4032 bytes (+88)

```py
info depth 1 score 40 time 0 nodes 20 pv b1c3
info depth 2 score 16 time 0 nodes 79 pv b1c3
info depth 3 score 36 time 1 nodes 602 nps 602000 pv b1c3
info depth 4 score 16 time 2 nodes 2263 nps 1131500 pv b1c3
info depth 5 score 4 time 6 nodes 16084 nps 2680666 pv b1c3
info depth 6 score 14 time 25 nodes 62502 nps 2500080 pv b1c3
info depth 7 score 10 time 163 nodes 465677 nps 2856914 pv b1c3
info depth 8 score 16 time 844 nodes 2130598 nps 2524405 pv b1c3
info depth 9 score 16 time 7201 nodes 19850163 nps 2756584 pv b1c3
info depth 10 score 16 time 46630 nodes 121079779 nps 2596606 pv b1c3
```

```py
Score of 4k.c-0.39 vs 4k.c-0.38: 294 - 167 - 539  [0.564] 1000
...      4k.c-0.39 playing White: 167 - 67 - 267  [0.600] 501
...      4k.c-0.39 playing Black: 127 - 100 - 272  [0.527] 499
...      White vs Black: 267 - 194 - 539  [0.536] 1000
Elo difference: 44.4 +/- 14.6, LOS: 100.0 %, DrawRatio: 53.9 %
```

### 0.40

Remove qsearch depth limit

32 bit: 5432 bytes (=)
64 bit: 4024 bytes (-8)

```py
info depth 1 score 40 time 0 nodes 20 pv b1c3
info depth 2 score 16 time 0 nodes 79 pv b1c3
info depth 3 score 36 time 0 nodes 602 pv b1c3
info depth 4 score 16 time 1 nodes 2263 nps 2263000 pv b1c3
info depth 5 score 4 time 6 nodes 16084 nps 2680666 pv b1c3
info depth 6 score 14 time 24 nodes 62502 nps 2604250 pv b1c3
info depth 7 score 10 time 161 nodes 466435 nps 2897111 pv b1c3
info depth 8 score 16 time 840 nodes 2140615 nps 2548351 pv b1c3
info depth 9 score 16 time 5246 nodes 14916338 nps 2843373 pv b1c3
info depth 10 score 16 time 37518 nodes 97307407 nps 2593619 pv b1c3
```

```py
Score of 4k.c-0.40 vs 4k.c-0.39: 522 - 458 - 1023  [0.516] 2003
...      4k.c-0.40 playing White: 310 - 181 - 511  [0.564] 1002
...      4k.c-0.40 playing Black: 212 - 277 - 512  [0.468] 1001
...      White vs Black: 587 - 393 - 1023  [0.548] 2003
Elo difference: 11.1 +/- 10.6, LOS: 98.0 %, DrawRatio: 51.1 %
```

### 0.41

Smaller PVS

32 bit: 5432 bytes (=)
64 bit: 4016 bytes (-8)

```py
info depth 1 score 40 time 0 nodes 20 pv b1c3
info depth 2 score 16 time 0 nodes 79 pv b1c3
info depth 3 score 36 time 0 nodes 602 pv b1c3
info depth 4 score 16 time 1 nodes 2263 nps 2263000 pv b1c3
info depth 5 score 4 time 6 nodes 16084 nps 2680666 pv b1c3
info depth 6 score 14 time 24 nodes 62502 nps 2604250 pv b1c3
info depth 7 score 10 time 160 nodes 466435 nps 2915218 pv b1c3
info depth 8 score 16 time 834 nodes 2140615 nps 2566684 pv b1c3
info depth 9 score 16 time 5193 nodes 14916338 nps 2872393 pv b1c3
info depth 10 score 16 time 37448 nodes 97307407 nps 2598467 pv b1c3
```

### 0.42

Reduce size

32 bit: 5432 bytes (=)
64 bit: 4000 bytes (-16)

```py
info depth 1 score 40 time 0 nodes 20 pv b1c3
info depth 2 score 16 time 0 nodes 79 pv b1c3
info depth 3 score 36 time 0 nodes 602 pv b1c3
info depth 4 score 16 time 1 nodes 2263 nps 2263000 pv b1c3
info depth 5 score 4 time 5 nodes 16084 nps 3216800 pv b1c3
info depth 6 score 14 time 24 nodes 62502 nps 2604250 pv b1c3
info depth 7 score 10 time 159 nodes 466435 nps 2933553 pv b1c3
info depth 8 score 16 time 836 nodes 2140615 nps 2560544 pv b1c3
info depth 9 score 16 time 5142 nodes 14916338 nps 2900882 pv b1c3
info depth 10 score 16 time 37123 nodes 97307407 nps 2621216 pv b1c3
```

### 0.43

Reduce size

32 bit: 5432 bytes (=)
64 bit: 3960 bytes (-40)

```py
info depth 1 score 40 time 1 nodes 20 nps 20000 pv b1c3
info depth 2 score 16 time 1 nodes 79 nps 79000 pv b1c3
info depth 3 score 36 time 1 nodes 602 nps 602000 pv b1c3
info depth 4 score 16 time 2 nodes 2263 nps 1131500 pv b1c3
info depth 5 score 4 time 6 nodes 16084 nps 2680666 pv b1c3
info depth 6 score 14 time 24 nodes 62502 nps 2604250 pv b1c3
info depth 7 score 10 time 158 nodes 466435 nps 2952120 pv b1c3
info depth 8 score 16 time 833 nodes 2140615 nps 2569765 pv b1c3
info depth 9 score 16 time 5213 nodes 14916338 nps 2861373 pv b1c3
info depth 10 score 16 time 37209 nodes 97307407 nps 2615157 pv b1c3
```

### 0.44

Reduce size with UCI hacks

32 bit: 5384 bytes (-48)
64 bit: 3904 bytes (-56)

```py
info depth 1 score 40 time 0 nodes 20 pv b1c3
info depth 2 score 16 time 0 nodes 79 pv b1c3
info depth 3 score 36 time 1 nodes 602 nps 602000 pv b1c3
info depth 4 score 16 time 1 nodes 2263 nps 2263000 pv b1c3
info depth 5 score 4 time 6 nodes 16084 nps 2680666 pv b1c3
info depth 6 score 14 time 24 nodes 62502 nps 2604250 pv b1c3
info depth 7 score 10 time 158 nodes 466435 nps 2952120 pv b1c3
info depth 8 score 16 time 834 nodes 2140615 nps 2566684 pv b1c3
info depth 9 score 16 time 5145 nodes 14916338 nps 2899191 pv b1c3
info depth 10 score 16 time 36699 nodes 97307407 nps 2651500 pv b1c3
```

### 0.45

Reduce size by abberviating authorship

32 bit: 5368 bytes (-16)
64 bit: 3888 bytes (-16)

```py
info depth 1 score 40 time 0 nodes 20 pv b1c3
info depth 2 score 16 time 0 nodes 79 pv b1c3
info depth 3 score 36 time 1 nodes 602 nps 602000 pv b1c3
info depth 4 score 16 time 1 nodes 2263 nps 2263000 pv b1c3
info depth 5 score 4 time 6 nodes 16084 nps 2680666 pv b1c3
info depth 6 score 14 time 24 nodes 62502 nps 2604250 pv b1c3
info depth 7 score 10 time 158 nodes 466435 nps 2952120 pv b1c3
info depth 8 score 16 time 834 nodes 2140615 nps 2566684 pv b1c3
info depth 9 score 16 time 5145 nodes 14916338 nps 2899191 pv b1c3
info depth 10 score 16 time 36699 nodes 97307407 nps 2651500 pv b1c3
```

### 0.46

Reduce size

32 bit: 5368 bytes (=)
64 bit: 3880 bytes (-8)

```py
info depth 1 score 40 time 0 nodes 20 pv b1c3
info depth 2 score 16 time 0 nodes 79 pv b1c3
info depth 3 score 36 time 0 nodes 602 pv b1c3
info depth 4 score 16 time 1 nodes 2263 nps 2263000 pv b1c3
info depth 5 score 4 time 6 nodes 16084 nps 2680666 pv b1c3
info depth 6 score 14 time 24 nodes 62502 nps 2604250 pv b1c3
info depth 7 score 10 time 163 nodes 466435 nps 2861564 pv b1c3
info depth 8 score 16 time 852 nodes 2140615 nps 2512458 pv b1c3
info depth 9 score 16 time 5336 nodes 14916338 nps 2795415 pv b1c3
info depth 10 score 16 time 37491 nodes 97307407 nps 2595487 pv b1c3
```

### 0.47

No RFP in PV nodes

32 bit: 5400 bytes (+32)
64 bit: 3888 bytes (+8)

```py
info depth 1 score 40 time 0 nodes 20 pv b1c3
info depth 2 score 16 time 0 nodes 79 pv b1c3
info depth 3 score 36 time 1 nodes 602 nps 602000 pv b1c3
info depth 4 score 16 time 1 nodes 2263 nps 2263000 pv b1c3
info depth 5 score 4 time 6 nodes 16070 nps 2678333 pv b1c3
info depth 6 score 14 time 25 nodes 62210 nps 2488400 pv b1c3
info depth 7 score 10 time 161 nodes 464048 nps 2882285 pv b1c3
info depth 8 score 16 time 838 nodes 2109355 nps 2517130 pv b1c3
info depth 9 score 16 time 5147 nodes 14722592 nps 2860421 pv b1c3
info depth 10 score 16 time 36669 nodes 94624750 nps 2580510 pv b1c3
```

```py
Score of 4k.c-0.47 vs 4k.c-0.46: 504 - 424 - 1072  [0.520] 2000
...      4k.c-0.47 playing White: 298 - 162 - 541  [0.568] 1001
...      4k.c-0.47 playing Black: 206 - 262 - 531  [0.472] 999
...      White vs Black: 560 - 368 - 1072  [0.548] 2000
Elo difference: 13.9 +/- 10.4, LOS: 99.6 %, DrawRatio: 53.6 %

Score of 4k.c-0.47 vs tscp1.82: 489 - 143 - 368  [0.673] 1000
...      4k.c-0.47 playing White: 261 - 50 - 189  [0.711] 500
...      4k.c-0.47 playing Black: 228 - 93 - 179  [0.635] 500
...      White vs Black: 354 - 278 - 368  [0.538] 1000
Elo difference: 125.4 +/- 17.5, LOS: 100.0 %, DrawRatio: 36.8 %

Score of 4k.c-0.47 vs stash11: 397 - 259 - 344  [0.569] 1000
...      4k.c-0.47 playing White: 252 - 79 - 170  [0.673] 501
...      4k.c-0.47 playing Black: 145 - 180 - 174  [0.465] 499
...      White vs Black: 432 - 224 - 344  [0.604] 1000
Elo difference: 48.3 +/- 17.5, LOS: 100.0 %, DrawRatio: 34.4 %
```

### 0.48

Protect from ply overflow

32 bit: 5416 bytes (+16)
64 bit: 3904 bytes (+16)

```py
info depth 1 score 40 time 0 nodes 20 pv b1c3
info depth 2 score 16 time 0 nodes 79 pv b1c3
info depth 3 score 36 time 0 nodes 602 pv b1c3
info depth 4 score 16 time 1 nodes 2263 nps 2263000 pv b1c3
info depth 5 score 4 time 6 nodes 16070 nps 2678333 pv b1c3
info depth 6 score 14 time 24 nodes 62210 nps 2592083 pv b1c3
info depth 7 score 10 time 163 nodes 464048 nps 2846920 pv b1c3
info depth 8 score 16 time 848 nodes 2109355 nps 2487446 pv b1c3
info depth 9 score 16 time 5237 nodes 14722592 nps 2811264 pv b1c3
info depth 10 score 16 time 36896 nodes 94624750 nps 2564634 pv b1c3
```

### 0.49

None = 0

32 bit: 5432 bytes (+16)
64 bit: 3912 bytes (+8)

```py
info depth 1 score 40 time 0 nodes 20 pv b1c3
info depth 2 score 16 time 0 nodes 79 pv b1c3
info depth 3 score 36 time 1 nodes 602 nps 602000 pv b1c3
info depth 4 score 16 time 1 nodes 2263 nps 2263000 pv b1c3
info depth 5 score 4 time 6 nodes 16070 nps 2678333 pv b1c3
info depth 6 score 14 time 26 nodes 62210 nps 2392692 pv b1c3
info depth 7 score 10 time 169 nodes 464048 nps 2745846 pv b1c3
info depth 8 score 16 time 879 nodes 2109355 nps 2399721 pv b1c3
info depth 9 score 16 time 5390 nodes 14722592 nps 2731464 pv b1c3
info depth 10 score 16 time 37885 nodes 94624750 nps 2497683 pv b1c3
```

```py
perft 6
info depth 6 nodes 119060324 time 15063 nps 7904157
```

### 0.50

History heuristic

32 bit: 5688 bytes (+256)
64 bit: 4088 bytes (+176)

```py
info depth 1 score 40 time 0 nodes 20 pv b1c3
info depth 2 score 16 time 0 nodes 79 pv b1c3
info depth 3 score 36 time 0 nodes 602 pv b1c3
info depth 4 score 16 time 1 nodes 2202 nps 2202000 pv b1c3
info depth 5 score 4 time 6 nodes 15474 nps 2579000 pv b1c3
info depth 6 score 14 time 21 nodes 52602 nps 2504857 pv b1c3
info depth 7 score 10 time 147 nodes 414984 nps 2823020 pv b1c3
info depth 8 score 16 time 683 nodes 1715831 nps 2512197 pv b1c3
info depth 9 score 16 time 4654 nodes 12612086 nps 2709945 pv b1c3
info depth 10 score 16 time 23165 nodes 58073555 nps 2506952 pv b1c3
```

```py
Score of 4k.c-0.50 vs 4k.c-0.48: 416 - 228 - 856  [0.563] 1500
...      4k.c-0.50 playing White: 238 - 92 - 420  [0.597] 750
...      4k.c-0.50 playing Black: 178 - 136 - 436  [0.528] 750
...      White vs Black: 374 - 270 - 856  [0.535] 1500
Elo difference: 43.8 +/- 11.5, LOS: 100.0 %, DrawRatio: 57.1 %

Score of 4k.c-0.50 vs stash11: 439 - 223 - 338  [0.608] 1000
...      4k.c-0.50 playing White: 253 - 71 - 176  [0.682] 500
...      4k.c-0.50 playing Black: 186 - 152 - 162  [0.534] 500
...      White vs Black: 405 - 257 - 338  [0.574] 1000
Elo difference: 76.2 +/- 17.7, LOS: 100.0 %, DrawRatio: 33.8 %

Score of 4k.c-0.50 vs stash12: 279 - 361 - 360  [0.459] 1000
...      4k.c-0.50 playing White: 173 - 132 - 195  [0.541] 500
...      4k.c-0.50 playing Black: 106 - 229 - 165  [0.377] 500
...      White vs Black: 402 - 238 - 360  [0.582] 1000
Elo difference: -28.6 +/- 17.2, LOS: 0.1 %, DrawRatio: 36.0 %
```

### 0.51

Reduce bytes and slight speedup

32 bit: 5672 bytes (-16)
64 bit: 4080 bytes (-8)

```py
info depth 1 score cp 40 time 0 nodes 20 pv b1c3
info depth 2 score cp 16 time 1 nodes 79 nps 79000 pv b1c3
info depth 3 score cp 36 time 1 nodes 602 nps 602000 pv b1c3
info depth 4 score cp 16 time 2 nodes 2202 nps 1101000 pv b1c3
info depth 5 score cp 4 time 6 nodes 15474 nps 2579000 pv b1c3
info depth 6 score cp 14 time 21 nodes 52602 nps 2504857 pv b1c3
info depth 7 score cp 10 time 138 nodes 414984 nps 3007130 pv b1c3
info depth 8 score cp 16 time 661 nodes 1715831 nps 2595810 pv b1c3
info depth 9 score cp 16 time 4507 nodes 12612086 nps 2798332 pv b1c3
info depth 10 score cp 16 time 22534 nodes 58073555 nps 2577152 pv b1c3
```

```py
Score of 4k.c-0.51 vs 4k.c-0.50: 733 - 633 - 1634  [0.517] 3000
...      4k.c-0.51 playing White: 443 - 265 - 792  [0.559] 1500
...      4k.c-0.51 playing Black: 290 - 368 - 842  [0.474] 1500
...      White vs Black: 811 - 555 - 1634  [0.543] 3000
Elo difference: 11.6 +/- 8.4, LOS: 99.7 %, DrawRatio: 54.5 %
```

### 0.52

Reduce bytes

32 bit: 5672 bytes (=)
64 bit: 4072 bytes (-8)

```py
info depth 1 score cp 40 time 0 nodes 20 pv b1c3
info depth 2 score cp 16 time 1 nodes 79 nps 79000 pv b1c3
info depth 3 score cp 36 time 1 nodes 602 nps 602000 pv b1c3
info depth 4 score cp 16 time 2 nodes 2202 nps 1101000 pv b1c3
info depth 5 score cp 4 time 6 nodes 15474 nps 2579000 pv b1c3
info depth 6 score cp 14 time 21 nodes 52602 nps 2504857 pv b1c3
info depth 7 score cp 10 time 138 nodes 414984 nps 3007130 pv b1c3
info depth 8 score cp 16 time 651 nodes 1715831 nps 2635685 pv b1c3
info depth 9 score cp 16 time 4394 nodes 12612086 nps 2870297 pv b1c3
info depth 10 score cp 16 time 22185 nodes 58073555 nps 2617694 pv b1c3
```

### 0.53

Slight speedup

32 bit: 5640 bytes (-16)
64 bit: 4072 bytes (=)

```py
info depth 1 score cp 40 time 0 nodes 20 pv b1c3
info depth 2 score cp 16 time 0 nodes 79 pv b1c3
info depth 3 score cp 36 time 1 nodes 602 nps 602000 pv b1c3
info depth 4 score cp 16 time 1 nodes 2202 nps 2202000 pv b1c3
info depth 5 score cp 4 time 6 nodes 15474 nps 2579000 pv b1c3
info depth 6 score cp 14 time 20 nodes 52602 nps 2630100 pv b1c3
info depth 7 score cp 10 time 135 nodes 414984 nps 3073955 pv b1c3
info depth 8 score cp 16 time 637 nodes 1715831 nps 2693612 pv b1c3
info depth 9 score cp 16 time 4310 nodes 12612086 nps 2926238 pv b1c3
info depth 10 score cp 16 time 21578 nodes 58073555 nps 2691331 pv b1c3
```

### 0.54

Reduce bytes

32 bit: 5608 bytes (-32)
64 bit: 4048 bytes (-24)

MD5: 884802623c504064c455cc1826802d02

```py
info depth 1 score cp 40 time 0 nodes 20 pv b1c3
info depth 2 score cp 16 time 0 nodes 79 pv b1c3
info depth 3 score cp 36 time 1 nodes 602 nps 602000 pv b1c3
info depth 4 score cp 16 time 1 nodes 2202 nps 2202000 pv b1c3
info depth 5 score cp 4 time 6 nodes 15474 nps 2579000 pv b1c3
info depth 6 score cp 14 time 21 nodes 52602 nps 2504857 pv b1c3
info depth 7 score cp 10 time 135 nodes 414984 nps 3073955 pv b1c3
info depth 8 score cp 16 time 636 nodes 1715831 nps 2697847 pv b1c3
info depth 9 score cp 16 time 4298 nodes 12612086 nps 2934408 pv b1c3
info depth 10 score cp 16 time 21407 nodes 58073555 nps 2712830 pv b1c3
```

```py
Score of 4k.c-0.54 vs 4k.c-0.53: 484 - 458 - 1058  [0.506] 2000
...      4k.c-0.54 playing White: 289 - 189 - 522  [0.550] 1000
...      4k.c-0.54 playing Black: 195 - 269 - 536  [0.463] 1000
...      White vs Black: 558 - 384 - 1058  [0.543] 2000
Elo difference: 4.5 +/- 10.4, LOS: 80.2 %, DrawRatio: 52.9 %
```

### 0.55

Less timing syscalls

32 bit: 5608 bytes (=)
64 bit: 4048 bytes (=)

MD5: 957f4b1b7d0bfb971915ca5eb31dd000

```py
info depth 1 score cp 40 time 0 nodes 20 pv b1c3
info depth 2 score cp 16 time 0 nodes 79 pv b1c3
info depth 3 score cp 36 time 0 nodes 602 pv b1c3
info depth 4 score cp 16 time 1 nodes 2202 nps 2202000 pv b1c3
info depth 5 score cp 4 time 5 nodes 15474 nps 3094800 pv b1c3
info depth 6 score cp 14 time 20 nodes 52602 nps 2630100 pv b1c3
info depth 7 score cp 10 time 134 nodes 414984 nps 3096895 pv b1c3
info depth 8 score cp 16 time 650 nodes 1715831 nps 2639740 pv b1c3
info depth 9 score cp 16 time 4297 nodes 12612086 nps 2935090 pv b1c3
info depth 10 score cp 16 time 21466 nodes 58073555 nps 2705373 pv b1c3
```

```py
Score of 4k.c-0.55 vs 4k.c-0.54: 467 - 448 - 1085  [0.505] 2000
...      4k.c-0.55 playing White: 285 - 187 - 529  [0.549] 1001
...      4k.c-0.55 playing Black: 182 - 261 - 556  [0.460] 999
...      White vs Black: 546 - 369 - 1085  [0.544] 2000
Elo difference: 3.3 +/- 10.3, LOS: 73.5 %, DrawRatio: 54.3 %
```

### 0.56

Reduce bytes

32 bit: 5608 bytes (=)
64 bit: 4040 bytes (-8)

MD5: 05d7423515fe8c7571d707b2d6f2fc21

```py
info depth 1 score cp 40 time 0 nodes 20 pv b1c3
info depth 2 score cp 16 time 0 nodes 79 pv b1c3
info depth 3 score cp 36 time 0 nodes 602 pv b1c3
info depth 4 score cp 16 time 1 nodes 2202 nps 2202000 pv b1c3
info depth 5 score cp 4 time 5 nodes 15474 nps 3094800 pv b1c3
info depth 6 score cp 14 time 19 nodes 52602 nps 2768526 pv b1c3
info depth 7 score cp 10 time 148 nodes 414984 nps 2803945 pv b1c3
info depth 8 score cp 16 time 671 nodes 1715831 nps 2557125 pv b1c3
info depth 9 score cp 16 time 4352 nodes 12612086 nps 2897997 pv b1c3
info depth 10 score cp 16 time 21101 nodes 58073555 nps 2752170 pv b1c3
```

### 0.57

Reduce bytes

32 bit: 5496 bytes (-112)
64 bit: 3976 bytes (-64)

MD5: cb7e8fa9d4dee2c95fd4407afa81c442

```py
info depth 1 score cp 40 time 0 nodes 20 pv b1c3
info depth 2 score cp 16 time 1 nodes 79 nps 79000 pv b1c3
info depth 3 score cp 36 time 1 nodes 602 nps 602000 pv b1c3
info depth 4 score cp 16 time 2 nodes 2202 nps 1101000 pv b1c3
info depth 5 score cp 4 time 6 nodes 15474 nps 2579000 pv b1c3
info depth 6 score cp 14 time 21 nodes 52602 nps 2504857 pv b1c3
info depth 7 score cp 10 time 139 nodes 414984 nps 2985496 pv b1c3
info depth 8 score cp 16 time 663 nodes 1715831 nps 2587980 pv b1c3
info depth 9 score cp 16 time 4467 nodes 12612086 nps 2823390 pv b1c3
info depth 10 score cp 16 time 22451 nodes 58073555 nps 2586680 pv b1c3
```

```py
perft 6
info depth 6 nodes 119060324 time 13032 nps 9135997
```

### 0.58

Late move reduction

32 bit: 5528 bytes (+32)
64 bit: 4048 bytes (+72)
Padding: 5 bytes

MD5: f61689e74dfe6399af80e955017e1cd4

```py
info depth 1 score cp 40 time 0 nodes 20 pv b1c3
info depth 2 score cp 16 time 1 nodes 79 nps 79000 pv b1c3
info depth 3 score cp 36 time 1 nodes 602 nps 602000 pv b1c3
info depth 4 score cp 16 time 1 nodes 1765 nps 1765000 pv b1c3
info depth 5 score cp 4 time 3 nodes 7187 nps 2395666 pv b1c3
info depth 6 score cp 14 time 9 nodes 25383 nps 2820333 pv b1c3
info depth 7 score cp 10 time 31 nodes 85777 nps 2767000 pv b1c3
info depth 8 score cp 16 time 107 nodes 311612 nps 2912261 pv b1c3
info depth 9 score cp 14 time 478 nodes 1340575 nps 2804550 pv b1c3
info depth 10 score cp 14 time 2090 nodes 5929872 nps 2837259 pv b1c3
info depth 11 score cp 13 time 13199 nodes 36760981 nps 2785133 pv b1c3
```

```py
Score of 4k.c-0.58 vs 4k.c-0.57: 271 - 152 - 577  [0.559] 1000
...      4k.c-0.58 playing White: 158 - 60 - 282  [0.598] 500
...      4k.c-0.58 playing Black: 113 - 92 - 295  [0.521] 500
...      White vs Black: 250 - 173 - 577  [0.538] 1000
Elo difference: 41.5 +/- 14.0, LOS: 100.0 %, DrawRatio: 57.7 %

Score of 4k.c-0.58 vs stash11: 483 - 170 - 347  [0.656] 1000
...      4k.c-0.58 playing White: 275 - 48 - 178  [0.727] 501
...      4k.c-0.58 playing Black: 208 - 122 - 169  [0.586] 499
...      White vs Black: 397 - 256 - 347  [0.571] 1000
Elo difference: 112.5 +/- 17.8, LOS: 100.0 %, DrawRatio: 34.7 %

Score of 4k.c-0.58 vs stash12: 667 - 597 - 741  [0.517] 2005
...      4k.c-0.58 playing White: 390 - 209 - 405  [0.590] 1004
...      4k.c-0.58 playing Black: 277 - 388 - 336  [0.445] 1001
...      White vs Black: 778 - 486 - 741  [0.573] 2005
Elo difference: 12.1 +/- 12.1, LOS: 97.6 %, DrawRatio: 37.0 %
```

### 0.59

Slight speedup

32 bit: 5608 bytes (+80)
64 bit: 4048 bytes (=)
Padding: 6 bytes

MD5: 71703f39997625c2146f654a5b1f143c

```py
info depth 1 score cp 40 time 0 nodes 20 pv b1c3
info depth 2 score cp 16 time 0 nodes 79 pv b1c3
info depth 3 score cp 36 time 0 nodes 602 pv b1c3
info depth 4 score cp 16 time 1 nodes 1765 nps 1765000 pv b1c3
info depth 5 score cp 4 time 3 nodes 7187 nps 2395666 pv b1c3
info depth 6 score cp 14 time 8 nodes 25383 nps 3172875 pv b1c3
info depth 7 score cp 10 time 28 nodes 85777 nps 3063464 pv b1c3
info depth 8 score cp 16 time 99 nodes 311612 nps 3147595 pv b1c3
info depth 9 score cp 14 time 441 nodes 1340575 nps 3039852 pv b1c3
info depth 10 score cp 14 time 2013 nodes 5929872 nps 2945788 pv b1c3
info depth 11 score cp 13 time 12683 nodes 36760981 nps 2898445 pv b1c3
```

### 0.60

Reduce bytes by assuming stdin never closes

32 bit: 5592 bytes (-16)
64 bit: 4024 bytes (-16)
Padding: 6 bytes

MD5: ce4d19e63b0329f8b0c85eeafce427c8

### 0.61

Reduce bytes by removing UCI info

32 bit: 5560 bytes (-32)
64 bit: 4000 bytes (-24)
Padding: 6 bytes

MD5: 509c5673c9a6679714fa47a979c5df50

### 0.62

Reduce bytes by smaller material table

32 bit: 5560 bytes (=)
64 bit: 3992 bytes (-8)
Padding: 3 bytes

MD5: 7b51e162375501dafa828a7dfeb4a5f9

### 0.63

Reduce bytes by changing C standard to gnu23

32 bit: 5560 bytes (=)
64 bit: 3984 bytes (-8)

Padding: 7 bytes

MD5: b5413110129ae62cdfd704d9f5de1ff6

### 0.64

Partial repetition detection

32 bit: 5640 bytes (+80)
64 bit: 4080 bytes (+88)

Padding: 6 bytes

MD5: 32665e47dddd77a8e6e11970fda617db

```py
info depth 1 score cp 40 time 0 nodes 20 pv b1c3
info depth 2 score cp 16 time 0 nodes 79 pv b1c3
info depth 3 score cp 36 time 0 nodes 602 pv b1c3
info depth 4 score cp 16 time 1 nodes 1765 nps 1765000 pv b1c3
info depth 5 score cp 4 time 3 nodes 7187 nps 2395666 pv b1c3
info depth 6 score cp 14 time 10 nodes 25383 nps 2538300 pv b1c3
info depth 7 score cp 10 time 33 nodes 85777 nps 2599303 pv b1c3
info depth 8 score cp 16 time 120 nodes 311635 nps 2596958 pv b1c3
info depth 9 score cp 14 time 541 nodes 1340528 nps 2477870 pv b1c3
info depth 10 score cp 14 time 3373 nodes 8195922 nps 2429861 pv b1c3
info depth 11 score cp 13 time 16834 nodes 40954740 nps 2432858 pv b1c3
```

```py
Score of 4k.c-0.64 vs 4k.c-0.63: 365 - 247 - 888  [0.539] 1500
...      4k.c-0.64 playing White: 216 - 97 - 437  [0.579] 750
...      4k.c-0.64 playing Black: 149 - 150 - 451  [0.499] 750
...      White vs Black: 366 - 246 - 888  [0.540] 1500
Elo difference: 27.4 +/- 11.2, LOS: 100.0 %, DrawRatio: 59.2 %
```

### 0.65

Reduce size by removing custom bool definition

32 bit: 5608 bytes (-32)
64 bit: 4064 bytes (-16)

Padding: 2 bytes

MD5: 801dd2c084ebb3a4cc37f2de076493b5

```py
info depth 1 score cp 40 time 0 nodes 20 pv b1c3
info depth 2 score cp 16 time 1 nodes 79 nps 79000 pv b1c3
info depth 3 score cp 36 time 1 nodes 602 nps 602000 pv b1c3
info depth 4 score cp 16 time 1 nodes 1765 nps 1765000 pv b1c3
info depth 5 score cp 4 time 4 nodes 7187 nps 1796750 pv b1c3
info depth 6 score cp 14 time 10 nodes 25383 nps 2538300 pv b1c3
info depth 7 score cp 10 time 33 nodes 85777 nps 2599303 pv b1c3
info depth 8 score cp 16 time 121 nodes 311635 nps 2575495 pv b1c3
info depth 9 score cp 14 time 540 nodes 1340528 nps 2482459 pv b1c3
info depth 10 score cp 14 time 3395 nodes 8195922 nps 2414115 pv b1c3
info depth 11 score cp 13 time 16999 nodes 40954740 nps 2409244 pv b1c3
```

```py
perft 6
info depth 6 nodes 119060324 time 13786 nps 8636321
```

```py
Score of 4k.c-0.64 vs stash12: 786 - 658 - 556  [0.532] 2000
...      4k.c-0.64 playing White: 470 - 236 - 294  [0.617] 1000
...      4k.c-0.64 playing Black: 316 - 422 - 262  [0.447] 1000
...      White vs Black: 892 - 552 - 556  [0.585] 2000
Elo difference: 22.3 +/- 12.9, LOS: 100.0 %, DrawRatio: 27.8 %
```

### 0.66

LMR adjustment by move count

32 bit: 5624 bytes (+16)
64 bit: 4056 bytes (-8)

Padding: 1 bytes

MD5: 8e38aa859d433d1c7d1faa7f73b93416

```
info depth 1 score cp 40 time 0 nodes 20 pv b1c3
info depth 2 score cp 16 time 0 nodes 79 pv b1c3
info depth 3 score cp 36 time 1 nodes 602 nps 602000 pv b1c3
info depth 4 score cp 16 time 1 nodes 1685 nps 1685000 pv b1c3
info depth 5 score cp 4 time 3 nodes 6930 nps 2310000 pv b1c3
info depth 6 score cp 14 time 9 nodes 22022 nps 2446888 pv b1c3
info depth 7 score cp 10 time 28 nodes 75478 nps 2695642 pv b1c3
info depth 8 score cp 16 time 95 nodes 259018 nps 2726505 pv b1c3
info depth 9 score cp 14 time 395 nodes 1046978 nps 2650577 pv b1c3
info depth 10 score cp 14 time 1754 nodes 4363840 nps 2487936 pv b1c3
info depth 11 score cp 13 time 11723 nodes 29067621 nps 2479537 pv b1c3
info depth 12 score cp 14 time 27521 nodes 68891493 nps 2503233 pv b1c3
```

```py
Score of 4k.c-0.66 vs 4k.c-0.65: 9210 - 8866 - 16924  [0.505] 35000
...      4k.c-0.66 playing White: 5653 - 3436 - 8411  [0.563] 17500
...      4k.c-0.66 playing Black: 3557 - 5430 - 8513  [0.446] 17500
...      White vs Black: 11083 - 6993 - 16924  [0.558] 35000
Elo difference: 3.4 +/- 2.6, LOS: 99.5 %, DrawRatio: 48.4 %
```

### 0.67

Reduce size by moving position history

32 bit: 5576 bytes (-48)
64 bit: 4032 bytes (-24)

Padding: 4 bytes

MD5: bd614583914cfc56d6c5e5b834e0791c

### 0.68

Reduce size by using a search stack

32 bit: 5560 bytes (-16)
64 bit: 4016 bytes (-16)

Padding: 7 bytes

MD5: 1c39c8895231105fa58932e4fd44001c

```py
info depth 1 score cp 40 time 0 nodes 20 pv b1c3
info depth 2 score cp 16 time 0 nodes 79 pv b1c3
info depth 3 score cp 36 time 0 nodes 602 pv b1c3
info depth 4 score cp 16 time 1 nodes 1685 nps 1685000 pv b1c3
info depth 5 score cp 4 time 3 nodes 6930 nps 2310000 pv b1c3
info depth 6 score cp 14 time 8 nodes 22022 nps 2752750 pv b1c3
info depth 7 score cp 10 time 26 nodes 75478 nps 2903000 pv b1c3
info depth 8 score cp 16 time 92 nodes 259018 nps 2815413 pv b1c3
info depth 9 score cp 14 time 373 nodes 1046978 nps 2806911 pv b1c3
info depth 10 score cp 14 time 1557 nodes 4363840 nps 2802723 pv b1c3
info depth 11 score cp 13 time 10664 nodes 29067621 nps 2725770 pv b1c3
info depth 12 score cp 14 time 25376 nodes 68891493 nps 2714828 pv b1c3
```

### 0.69

Reduce size by offloading move list to search stack

32 bit: 5480 bytes (-80)
64 bit: 3992 bytes (-24)

Padding: 1 bytes

MD5: 8abad2145c109e6cecc1bdd441772e6d

```py
info depth 1 score cp 40 time 0 nodes 20 pv b1c3
info depth 2 score cp 16 time 0 nodes 79 pv b1c3
info depth 3 score cp 36 time 1 nodes 602 nps 602000 pv b1c3
info depth 4 score cp 16 time 1 nodes 1685 nps 1685000 pv b1c3
info depth 5 score cp 4 time 3 nodes 6930 nps 2310000 pv b1c3
info depth 6 score cp 14 time 8 nodes 22022 nps 2752750 pv b1c3
info depth 7 score cp 10 time 26 nodes 75478 nps 2903000 pv b1c3
info depth 8 score cp 16 time 88 nodes 259018 nps 2943386 pv b1c3
info depth 9 score cp 14 time 369 nodes 1046978 nps 2837338 pv b1c3
info depth 10 score cp 14 time 1589 nodes 4363840 nps 2746280 pv b1c3
info depth 11 score cp 13 time 10821 nodes 29067621 nps 2686223 pv b1c3
info depth 12 score cp 14 time 25865 nodes 68891493 nps 2663502 pv b1c3
```

```py
Score of 4k.c-0.69 vs 4k.c-0.50: 387 - 118 - 495  [0.634] 1000
...      4k.c-0.69 playing White: 229 - 37 - 234  [0.692] 500
...      4k.c-0.69 playing Black: 158 - 81 - 261  [0.577] 500
...      White vs Black: 310 - 195 - 495  [0.557] 1000
Elo difference: 95.8 +/- 15.3, LOS: 100.0 %, DrawRatio: 49.5 %

Score of 4k.c-0.69 vs stash12: 409 - 295 - 296  [0.557] 1000
...      4k.c-0.69 playing White: 241 - 103 - 156  [0.638] 500
...      4k.c-0.69 playing Black: 168 - 192 - 140  [0.476] 500
...      White vs Black: 433 - 271 - 296  [0.581] 1000
Elo difference: 39.8 +/- 18.1, LOS: 100.0 %, DrawRatio: 29.6 %

Score of 4k.c-0.69 vs stash13: 512 - 551 - 437  [0.487] 1500
...      4k.c-0.69 playing White: 303 - 219 - 227  [0.556] 749
...      4k.c-0.69 playing Black: 209 - 332 - 210  [0.418] 751
...      White vs Black: 635 - 428 - 437  [0.569] 1500
Elo difference: -9.0 +/- 14.8, LOS: 11.6 %, DrawRatio: 29.1 %
```

### 0.70

Reduce size by restricting move string

32 bit: 5512 bytes (+32)
64 bit: 3984 bytes (-8)

Padding: 6 bytes

MD5: bfb6e6419b23fecd56b6f090db91de55

```py
info depth 1 score cp 40 time 0 nodes 20 pv b1c3
info depth 2 score cp 16 time 0 nodes 79 pv b1c3
info depth 3 score cp 36 time 0 nodes 602 pv b1c3
info depth 4 score cp 16 time 1 nodes 1685 nps 1685000 pv b1c3
info depth 5 score cp 4 time 3 nodes 6930 nps 2310000 pv b1c3
info depth 6 score cp 14 time 7 nodes 22022 nps 3146000 pv b1c3
info depth 7 score cp 10 time 25 nodes 75478 nps 3019120 pv b1c3
info depth 8 score cp 16 time 85 nodes 259018 nps 3047270 pv b1c3
info depth 9 score cp 14 time 346 nodes 1046978 nps 3025947 pv b1c3
info depth 10 score cp 14 time 1490 nodes 4363840 nps 2928751 pv b1c3
info depth 11 score cp 13 time 10195 nodes 29067621 nps 2851164 pv b1c3
info depth 12 score cp 14 time 24074 nodes 68891493 nps 2861655 pv b1c3
```

```py
Score of 4k.c-0.70 vs 4k.c-0.69: 259 - 276 - 465  [0.491] 1000
...      4k.c-0.70 playing White: 171 - 104 - 225  [0.567] 500
...      4k.c-0.70 playing Black: 88 - 172 - 240  [0.416] 500
...      White vs Black: 343 - 192 - 465  [0.576] 1000
Elo difference: -5.9 +/- 15.7, LOS: 23.1 %, DrawRatio: 46.5 %
```

### 0.71

Reduce size by restricting every pointer possible

32 bit: 5464 bytes (-48)
64 bit: 3968 bytes (-16)

Padding: 1 byte

MD5: bac03daf8711631b797111f8306a698c

```py
info depth 1 score cp 40 time 0 nodes 20 pv b1c3
info depth 2 score cp 16 time 1 nodes 79 nps 79000 pv b1c3
info depth 3 score cp 36 time 1 nodes 602 nps 602000 pv b1c3
info depth 4 score cp 16 time 1 nodes 1685 nps 1685000 pv b1c3
info depth 5 score cp 4 time 3 nodes 6930 nps 2310000 pv b1c3
info depth 6 score cp 14 time 8 nodes 22022 nps 2752750 pv b1c3
info depth 7 score cp 10 time 27 nodes 75478 nps 2795481 pv b1c3
info depth 8 score cp 16 time 90 nodes 259018 nps 2877977 pv b1c3
info depth 9 score cp 14 time 366 nodes 1046978 nps 2860595 pv b1c3
info depth 10 score cp 14 time 1540 nodes 4363840 nps 2833662 pv b1c3
info depth 11 score cp 13 time 10389 nodes 29067621 nps 2797922 pv b1c3
info depth 12 score cp 14 time 24624 nodes 68891493 nps 2797737 pv b1c3
info depth 13 score cp 12 time 112215 nodes 317675226 nps 2830951 pv e2e3
```

```py
perft 6
info depth 6 nodes 119060324 time 11123 nps 10703975
```

```py
Score of 4k.c-0.71 vs 4k.c-0.70: 261 - 254 - 485  [0.503] 1000
...      4k.c-0.71 playing White: 162 - 88 - 251  [0.574] 501
...      4k.c-0.71 playing Black: 99 - 166 - 234  [0.433] 499
...      White vs Black: 328 - 187 - 485  [0.571] 1000
Elo difference: 2.4 +/- 15.4, LOS: 62.1 %, DrawRatio: 48.5 %
```

### 0.72

Full repetition detection

32 bit: 5544 bytes (+80)
64 bit: 4048 bytes (+80)

Padding: 0 bytes

MD5: 75594d64e663f035759db3c16e905a3b

```py
info depth 1 score cp 40 time 0 nodes 20 pv b1c3
info depth 2 score cp 16 time 0 nodes 79 pv b1c3
info depth 3 score cp 36 time 0 nodes 602 pv b1c3
info depth 4 score cp 16 time 1 nodes 1685 nps 1685000 pv b1c3
info depth 5 score cp 4 time 2 nodes 6930 nps 3465000 pv b1c3
info depth 6 score cp 14 time 8 nodes 22022 nps 2752750 pv b1c3
info depth 7 score cp 10 time 25 nodes 75478 nps 3019120 pv b1c3
info depth 8 score cp 16 time 83 nodes 259018 nps 3120698 pv b1c3
info depth 9 score cp 14 time 342 nodes 1046978 nps 3061339 pv b1c3
info depth 10 score cp 14 time 1458 nodes 4363840 nps 2993031 pv b1c3
info depth 11 score cp 13 time 9752 nodes 29067621 nps 2980683 pv b1c3
info depth 12 score cp 14 time 23245 nodes 68891493 nps 2963712 pv b1c3
info depth 13 score cp 12 time 106806 nodes 317675226 nps 2974320 pv e2e3
```

```py
Score of 4k.c-0.72 vs 4k.c-0.71: 1221 - 1006 - 1773  [0.527] 4000
...      4k.c-0.72 playing White: 749 - 421 - 830  [0.582] 2000
...      4k.c-0.72 playing Black: 472 - 585 - 943  [0.472] 2000
...      White vs Black: 1334 - 893 - 1773  [0.555] 4000
Elo difference: 18.7 +/- 8.0, LOS: 100.0 %, DrawRatio: 44.3 %
```

### 0.73

No repetition detection on negative depths

32 bit: 5544 bytes (=)
64 bit: 4056 bytes (+8)

Padding: 1 byte

MD5: f6202951e77231c1f3080e801ab2d4f2

```
info depth 1 score cp 40 time 0 nodes 20 pv b1c3
info depth 2 score cp 16 time 0 nodes 79 pv b1c3
info depth 3 score cp 36 time 0 nodes 602 pv b1c3
info depth 4 score cp 16 time 0 nodes 1685 pv b1c3
info depth 5 score cp 4 time 2 nodes 6930 nps 3465000 pv b1c3
info depth 6 score cp 14 time 7 nodes 22022 nps 3146000 pv b1c3
info depth 7 score cp 10 time 25 nodes 75478 nps 3019120 pv b1c3
info depth 8 score cp 16 time 83 nodes 259018 nps 3120698 pv b1c3
info depth 9 score cp 14 time 344 nodes 1046978 nps 3043540 pv b1c3
info depth 10 score cp 14 time 1425 nodes 4363840 nps 3062343 pv b1c3
info depth 11 score cp 13 time 9517 nodes 29067621 nps 3054284 pv b1c3
info depth 12 score cp 14 time 22549 nodes 68891493 nps 3055190 pv b1c3
info depth 13 score cp 12 time 104563 nodes 317675226 nps 3038122 pv e2e3
```

```py
Score of 4k.c-0.73 vs 4k.c-0.72: 24177 - 18685 - 20138  [0.544] 63000
...      4k.c-0.73 playing White: 14362 - 7346 - 9793  [0.611] 31501
...      4k.c-0.73 playing Black: 9815 - 11339 - 10345  [0.476] 31499
...      White vs Black: 25701 - 17161 - 20138  [0.568] 63000
Elo difference: 30.4 +/- 2.2, LOS: 100.0 %, DrawRatio: 32.0 %

Score of 4k.c-0.73 vs stash12: 1448 - 1065 - 487  [0.564] 3000
...      4k.c-0.73 playing White: 851 - 415 - 233  [0.645] 1499
...      4k.c-0.73 playing Black: 597 - 650 - 254  [0.482] 1501
...      White vs Black: 1501 - 1012 - 487  [0.582] 3000
Elo difference: 44.6 +/- 11.4, LOS: 100.0 %, DrawRatio: 16.2 %

Score of 4k.c-0.73 vs stash13: 1254 - 1257 - 489  [0.499] 3000
...      4k.c-0.73 playing White: 765 - 486 - 251  [0.593] 1502
...      4k.c-0.73 playing Black: 489 - 771 - 238  [0.406] 1498
...      White vs Black: 1536 - 975 - 489  [0.594] 3000
Elo difference: -0.3 +/- 11.4, LOS: 47.6 %, DrawRatio: 16.3 %
```

### 0.74

Retune eval, remove quantization

32 bit: 5544 bytes (=)
64 bit: 4064 bytes (+8)

Padding: 5 bytes

MD5: 375f6992479587ebd09d7e5fb8bdec7f

```py
info depth 1 score cp 28 time 0 nodes 20 pv b1c3
info depth 2 score cp 12 time 0 nodes 79 pv b1c3
info depth 3 score cp 26 time 0 nodes 602 pv b1c3
info depth 4 score cp 12 time 1 nodes 1613 nps 1613000 pv b1c3
info depth 5 score cp 2 time 2 nodes 7290 nps 3645000 pv b1c3
info depth 6 score cp 12 time 6 nodes 21491 nps 3581833 pv b1c3
info depth 7 score cp 9 time 21 nodes 72224 nps 3439238 pv b1c3
info depth 8 score cp 12 time 70 nodes 241422 nps 3448885 pv b1c3
info depth 9 score cp 8 time 553 nodes 1775163 nps 3210059 pv g1f3
info depth 10 score cp 12 time 1418 nodes 4499363 nps 3173034 pv g1f3
info depth 11 score cp 10 time 6518 nodes 20362006 nps 3123965 pv g1f3
info depth 12 score cp 11 time 27133 nodes 81138458 nps 2990397 pv g1f3
info depth 13 score cp 8 time 81017 nodes 245873493 nps 3034838 pv g1f3
```

```py
Score of 4k.c-0.74 vs 4k.c-0.73: 597 - 448 - 455  [0.550] 1500
...      4k.c-0.74 playing White: 358 - 168 - 224  [0.627] 750
...      4k.c-0.74 playing Black: 239 - 280 - 231  [0.473] 750
...      White vs Black: 638 - 407 - 455  [0.577] 1500
Elo difference: 34.6 +/- 14.7, LOS: 100.0 %, DrawRatio: 30.3 %

Score of 4k.c-0.74 vs stash13: 819 - 852 - 329  [0.492] 2000
...      4k.c-0.74 playing White: 487 - 347 - 165  [0.570] 999
...      4k.c-0.74 playing Black: 332 - 505 - 164  [0.414] 1001
...      White vs Black: 992 - 679 - 329  [0.578] 2000
Elo difference: -5.7 +/- 13.9, LOS: 21.0 %, DrawRatio: 16.4 %
```

### 0.75

Do LMR earlier

32 bit: 5544 bytes (=)
64 bit: 4064 bytes (=)

Padding: 6 bytes

MD5: 5a4e474242d00ce1bd073d40ea246220

```py
info depth 1 score cp 28 time 0 nodes 20 pv b1c3
info depth 2 score cp 12 time 0 nodes 66 pv b1c3
info depth 3 score cp 26 time 0 nodes 267 pv b1c3
info depth 4 score cp 12 time 0 nodes 871 pv b1c3
info depth 5 score cp 2 time 1 nodes 3113 nps 3113000 pv b1c3
info depth 6 score cp 12 time 3 nodes 9437 nps 3145666 pv b1c3
info depth 7 score cp 8 time 13 nodes 42899 nps 3299923 pv d2d3
info depth 8 score cp 12 time 59 nodes 192036 nps 3254847 pv g1f3
info depth 9 score cp 8 time 157 nodes 512900 nps 3266878 pv g1f3
info depth 10 score cp 11 time 485 nodes 1549651 nps 3195156 pv g1f3
info depth 11 score cp 10 time 1679 nodes 5333481 nps 3176581 pv g1f3
info depth 12 score cp 7 time 13651 nodes 42384026 nps 3104829 pv b1c3
info depth 13 score cp 8 time 33612 nodes 104152913 nps 3098682 pv b1c3
info depth 14 score cp 7 time 155412 nodes 481602608 nps 3098876 pv b1c3
```

```py
Score of 4k.c-0.75 vs 4k.c-0.74: 2040 - 1762 - 1698  [0.525] 5500
...      4k.c-0.75 playing White: 1206 - 699 - 846  [0.592] 2751
...      4k.c-0.75 playing Black: 834 - 1063 - 852  [0.458] 2749
...      White vs Black: 2269 - 1533 - 1698  [0.567] 5500
Elo difference: 17.6 +/- 7.6, LOS: 100.0 %, DrawRatio: 30.9 %

Score of 4k.c-0.75 vs stash13: 890 - 776 - 334  [0.528] 2000
...      4k.c-0.75 playing White: 550 - 293 - 157  [0.628] 1000
...      4k.c-0.75 playing Black: 340 - 483 - 177  [0.428] 1000
...      White vs Black: 1033 - 633 - 334  [0.600] 2000
Elo difference: 19.8 +/- 13.9, LOS: 99.7 %, DrawRatio: 16.7 %
```

### 0.76

Adjust LMR by being in a zero-window

32 bit: 5560 bytes (+16)
64 bit: 4072 bytes (+8)

Padding: 1 byte

MD5: 821764cf77a79e30e5de74f0eb95c9b6

```py
info depth 1 score cp 28 time 0 nodes 20 pv b1c3
info depth 2 score cp 12 time 0 nodes 76 pv b1c3
info depth 3 score cp 26 time 0 nodes 515 pv b1c3
info depth 4 score cp 12 time 1 nodes 1457 nps 1457000 pv b1c3
info depth 5 score cp 2 time 2 nodes 5610 nps 2805000 pv b1c3
info depth 6 score cp 12 time 6 nodes 17761 nps 2960166 pv b1c3
info depth 7 score cp 9 time 18 nodes 59582 nps 3310111 pv b1c3
info depth 8 score cp 12 time 63 nodes 202325 nps 3211507 pv b1c3
info depth 9 score cp 8 time 245 nodes 754598 nps 3079991 pv b1c3
info depth 10 score cp 11 time 1327 nodes 4016822 nps 3026994 pv g1f3
info depth 11 score cp 10 time 5939 nodes 17722949 nps 2984163 pv d2d4
info depth 12 score cp 9 time 25373 nodes 76445512 nps 3012868 pv b1c3
info depth 13 score cp 7 time 116670 nodes 351349101 nps 3011477 pv g1f3
```

```py
Score of 4k.c-0.76 vs 4k.c-0.75: 28136 - 27167 - 24697  [0.506] 80000
...      4k.c-0.76 playing White: 16956 - 10711 - 12333  [0.578] 40000
...      4k.c-0.76 playing Black: 11180 - 16456 - 12364  [0.434] 40000
...      White vs Black: 33412 - 21891 - 24697  [0.572] 80000
Elo difference: 4.2 +/- 2.0, LOS: 100.0 %, DrawRatio: 30.9 %

Score of 4k.c-0.76 vs 4k.c-0.69: 677 - 500 - 823  [0.544] 2000
...      4k.c-0.76 playing White: 425 - 196 - 380  [0.614] 1001
...      4k.c-0.76 playing Black: 252 - 304 - 443  [0.474] 999
...      White vs Black: 729 - 448 - 823  [0.570] 2000
Elo difference: 30.8 +/- 11.7, LOS: 100.0 %, DrawRatio: 41.1 %

Score of 4k.c-0.76 vs stash12: 1011 - 675 - 314  [0.584] 2000
...      4k.c-0.76 playing White: 601 - 263 - 136  [0.669] 1000
...      4k.c-0.76 playing Black: 410 - 412 - 178  [0.499] 1000
...      White vs Black: 1013 - 673 - 314  [0.585] 2000
Elo difference: 58.9 +/- 14.1, LOS: 100.0 %, DrawRatio: 15.7 %

Score of 4k.c-0.76 vs stash13: 1315 - 1180 - 505  [0.522] 3000
...      4k.c-0.76 playing White: 762 - 503 - 235  [0.586] 1500
...      4k.c-0.76 playing Black: 553 - 677 - 270  [0.459] 1500
...      White vs Black: 1439 - 1056 - 505  [0.564] 3000
Elo difference: 15.6 +/- 11.3, LOS: 99.7 %, DrawRatio: 16.8 %

Score of 4k.c-0.76 vs vice1.1: 935 - 761 - 304  [0.543] 2000
...      4k.c-0.76 playing White: 567 - 304 - 130  [0.631] 1001
...      4k.c-0.76 playing Black: 368 - 457 - 174  [0.455] 999
...      White vs Black: 1024 - 672 - 304  [0.588] 2000
Elo difference: 30.3 +/- 14.1, LOS: 100.0 %, DrawRatio: 15.2 %
```

0.69 vs dev:
```py
Elo   | -44.88 +- 11.48 (95%)
Conf  | 10.0+0.10s Threads=1 Hash=1MB
Games | N: 2024 W: 507 L: 767 D: 750
Penta | [126, 288, 372, 172, 54]
https://gedas.pythonanywhere.com/test/34/
```

### 0.77

Reduce size by reordering position struct

32 bit: 5560 bytes (=)
64 bit: 4032 bytes (-40)

MD5: 7cf51543e3bdf9a834494bf3621e65cf

```py
Fast:
Elo   | -9.52 +- 8.45 (95%)
SPRT  | 10.0+0.10s Threads=1 Hash=1MB
LLR   | -1.39 (-2.94, 2.94) [-5.00, 0.00]
Games | N: 2812 W: 981 L: 1058 D: 773
Penta | [100, 256, 761, 199, 90]
https://gedas.pythonanywhere.com/test/40/

Slow:
Elo   | -8.16 +- 7.00 (95%)
SPRT  | 10.0+0.10s Threads=1 Hash=1MB
LLR   | -1.60 (-2.94, 2.94) [-5.00, 0.00]
Games | N: 3578 W: 1269 L: 1353 D: 956
Penta | [119, 269, 1070, 239, 92]
https://gedas.pythonanywhere.com/test/41/
```

### 0.78

Sided history table

32 bit: 5592 bytes (+32)
64 bit: 4056 bytes (+16)

MD5: 7ddd99279e31e9419432858b91d9dd26

```py
info depth 1 score cp 28 time 0 nodes 20 pv b1c3
info depth 2 score cp 12 time 0 nodes 76 pv b1c3
info depth 3 score cp 26 time 0 nodes 516 pv b1c3
info depth 4 score cp 12 time 1 nodes 1446 nps 1446000 pv b1c3
info depth 5 score cp 2 time 1 nodes 5649 nps 5649000 pv b1c3
info depth 6 score cp 12 time 4 nodes 17159 nps 4289750 pv b1c3
info depth 7 score cp 9 time 12 nodes 59088 nps 4924000 pv b1c3
info depth 8 score cp 12 time 41 nodes 203282 nps 4958097 pv b1c3
info depth 9 score cp 10 time 226 nodes 1061354 nps 4696256 pv e2e4
info depth 10 score cp 9 time 631 nodes 2915915 nps 4621101 pv e2e4
info depth 11 score cp 10 time 2852 nodes 13253579 nps 4647117 pv b1c3
```

```py
Elo   | 16.12 +- 8.58 (95%)
SPRT  | 10.0+0.10s Threads=1 Hash=1MB
LLR   | 3.02 (-2.94, 2.94) [0.00, 5.00]
Games | N: 4270 W: 1639 L: 1441 D: 1190
Penta | [226, 382, 780, 462, 285]
https://gedas.pythonanywhere.com/test/42/
```

### 0.78

Set min time control to /48

32 bit: 5592 bytes (=)
64 bit: 4056 bytes (=)

MD5: 232384a4001b4b40cf32cd608a7aa374

```py
Elo   | 11.04 +- 6.34 (95%)
SPRT  | 10.0+0.10s Threads=1 Hash=1MB
LLR   | 3.20 (-2.94, 2.94) [0.00, 5.00]
Games | N: 6076 W: 2296 L: 2103 D: 1677
Penta | [241, 488, 1437, 581, 291]
https://gedas.pythonanywhere.com/test/45/
```

### 0.79

Set min time control to /32, max time control to /2

32 bit: 5592 bytes (=)
64 bit: 4056 bytes (=)

MD5: 2a9fb73b809cdd36a7865b03e8161b57

```py
Elo   | 9.35 +- 6.09 (95%)
SPRT  | 10.0+0.10s Threads=1 Hash=1MB
LLR   | 2.98 (-2.94, 2.94) [0.00, 5.00]
Games | N: 7504 W: 2812 L: 2610 D: 2082
Penta | [356, 652, 1607, 708, 429]
https://gedas.pythonanywhere.com/test/48/
```

### 0.80

Reduce size by optimizing history updating

32 bit: 5560 bytes (-32)
64 bit: 4048 bytes (-8)

MD5: 06f00d758f30c12820d7143972029894

### 0.81

Noticed some losses on time so revert super aggressive time control

64 bit: 4056 bytes (+8)

MD5: 2f814dc16daf5ed1fc69c0fb01e4f154

### 0.82

Reduce size by removing blockers for king

64 bit: 4048 bytes (-8)

MD5: faf57ec95bd24eb1e70bb66cac734125

### 0.83

Retune eval for 16 tempo

64 bit: 4048 bytes (=)

MD5: ed877c31f6e76028c56368b5af5af804

```py
info depth 1 score cp 24 time 0 nodes 20 pv b1c3
info depth 2 score cp 16 time 0 nodes 76 pv b1c3
info depth 3 score cp 22 time 0 nodes 516 pv b1c3
info depth 4 score cp 16 time 0 nodes 1439 pv b1c3
info depth 5 score cp -2 time 1 nodes 5638 nps 5638000 pv b1c3
info depth 6 score cp 3 time 5 nodes 17858 nps 3571600 pv b1c3
info depth 7 score cp 4 time 13 nodes 60654 nps 4665692 pv b1c3
info depth 8 score cp 16 time 42 nodes 214427 nps 5105404 pv b1c3
info depth 9 score cp 7 time 193 nodes 970730 nps 5029689 pv b1c3
info depth 10 score cp 9 time 575 nodes 2837511 nps 4934801 pv b1c3
info depth 11 score cp 12 time 2421 nodes 11729785 nps 4845016 pv b1c3
bestmove b1c3
11729785 nodes 4845016 nps
```

```py
Elo   | 4.84 +- 3.84 (95%)
SPRT  | 10.0+0.10s Threads=1 Hash=1MB
LLR   | 2.99 (-2.94, 2.94) [0.00, 5.00]
Games | N: 21594 W: 7986 L: 7685 D: 5923
Penta | [1262, 2114, 3848, 2207, 1366]
https://gedas.pythonanywhere.com/test/53/
```

### 0.84

Reverse bitboards

64 bit: 4096 bytes (+48)

MD5: cecf3ca2861b208f20fa5af4d1115656

(This speed difference is much more pronounced on mini build)
```py
info depth 1 score cp 24 time 0 nodes 20 pv b1c3
info depth 2 score cp 16 time 0 nodes 76 pv b1c3
info depth 3 score cp 22 time 0 nodes 516 pv b1c3
info depth 4 score cp 16 time 0 nodes 1439 pv b1c3
info depth 5 score cp -2 time 1 nodes 5638 nps 5638000 pv b1c3
info depth 6 score cp 3 time 3 nodes 17858 nps 5952666 pv b1c3
info depth 7 score cp 4 time 9 nodes 60654 nps 6739333 pv b1c3
info depth 8 score cp 16 time 34 nodes 214427 nps 6306676 pv b1c3
info depth 9 score cp 7 time 161 nodes 970730 nps 6029378 pv b1c3
info depth 10 score cp 9 time 479 nodes 2837511 nps 5923822 pv b1c3
info depth 11 score cp 12 time 2042 nodes 11729785 nps 5744262 pv b1c3
bestmove b1c3
11729785 nodes 5744262 nps
```

```py
Elo   | 43.28 +- 14.57 (95%)
SPRT  | 10.0+0.10s Threads=1 Hash=1MB
LLR   | 2.95 (-2.94, 2.94) [0.00, 5.00]
Games | N: 1396 W: 594 L: 421 D: 381
Penta | [53, 95, 283, 160, 107]
https://gedas.pythonanywhere.com/test/115/
```

```py
Score of 4k.c-0.84 vs stash12: 579 - 273 - 148  [0.653] 1000
...      4k.c-0.84 playing White: 329 - 105 - 66  [0.724] 500
...      4k.c-0.84 playing Black: 250 - 168 - 82  [0.582] 500
...      White vs Black: 497 - 355 - 148  [0.571] 1000
Elo difference: 109.8 +/- 20.7, LOS: 100.0 %, DrawRatio: 14.8 %

Score of 4k.c-0.84 vs stash13: 510 - 338 - 152  [0.586] 1000
...      4k.c-0.84 playing White: 303 - 135 - 62  [0.668] 500
...      4k.c-0.84 playing Black: 207 - 203 - 90  [0.504] 500
...      White vs Black: 506 - 342 - 152  [0.582] 1000
Elo difference: 60.4 +/- 20.1, LOS: 100.0 %, DrawRatio: 15.2 %
```

### 0.85

Retune eval with K=2.7

64 bit: 4096 bytes (=)

MD5: 75ad68ea998fcb150a3a9a03433af149

```py
info depth 1 score cp 28 time 0 nodes 20 pv b1c3
info depth 2 score cp 16 time 0 nodes 76 pv b1c3
info depth 3 score cp 26 time 0 nodes 516 pv b1c3
info depth 4 score cp 16 time 1 nodes 1446 nps 1446000 pv b1c3
info depth 5 score cp -1 time 1 nodes 5645 nps 5645000 pv b1c3
info depth 6 score cp 1 time 3 nodes 17715 nps 5905000 pv b1c3
info depth 7 score cp 7 time 10 nodes 59645 nps 5964500 pv b1c3
info depth 8 score cp 16 time 35 nodes 203957 nps 5827342 pv b1c3
info depth 9 score cp 9 time 193 nodes 1083548 nps 5614238 pv b1c3
info depth 10 score cp 13 time 639 nodes 3556915 nps 5566377 pv b1c3
info depth 11 score cp 9 time 3234 nodes 17643223 nps 5455542 pv e2e4
bestmove e2e4
17643223 nodes 5455542 nps
```

```py
Elo   | 4.44 +- 3.54 (95%)
SPRT  | 10.0+0.10s Threads=1 Hash=1MB
LLR   | 3.01 (-2.94, 2.94) [0.00, 5.00]
Games | N: 25068 W: 9329 L: 9009 D: 6730
Penta | [1435, 2420, 4660, 2428, 1591]
https://gedas.pythonanywhere.com/test/122/
```

### 0.86

Reduce bytes by not flipping position in eval

64 bit: 4080 bytes (-16)

MD5: c054c12c4605c8821e3a2b3ccaca63ad

```py
info depth 1 score cp 28 time 0 nodes 20 pv b1c3
info depth 2 score cp 16 time 0 nodes 76 pv b1c3
info depth 3 score cp 26 time 0 nodes 516 pv b1c3
info depth 4 score cp 16 time 0 nodes 1446 pv b1c3
info depth 5 score cp -1 time 1 nodes 5645 nps 5645000 pv b1c3
info depth 6 score cp 1 time 3 nodes 17715 nps 5905000 pv b1c3
info depth 7 score cp 7 time 10 nodes 59645 nps 5964500 pv b1c3
info depth 8 score cp 16 time 33 nodes 203957 nps 6180515 pv b1c3
info depth 9 score cp 9 time 193 nodes 1083548 nps 5614238 pv b1c3
info depth 10 score cp 13 time 632 nodes 3556915 nps 5628030 pv b1c3
info depth 11 score cp 9 time 3194 nodes 17643223 nps 5523864 pv e2e4
bestmove e2e4
17643223 nodes 5523864 nps
```

```py
Elo   | -4.54 +- 3.49 (95%)
SPRT  | 10.0+0.10s Threads=1 Hash=1MB
LLR   | -2.96 (-2.94, 2.94) [-5.00, 0.00]
Games | N: 15290 W: 5494 L: 5694 D: 4102
Penta | [518, 1136, 4493, 1024, 474]
https://gedas.pythonanywhere.com/test/128/
```

### 0.87

Reduce size with a hack for position flipping

64 bit: 4072 bytes (-8)
Fill: 3

MD5: 6284e220ed1575530e1227427c1bdef2

```py
info depth 1 score cp 28 time 0 nodes 20 pv b1c3
info depth 2 score cp 16 time 0 nodes 76 pv b1c3
info depth 3 score cp 26 time 0 nodes 516 pv b1c3
info depth 4 score cp 16 time 0 nodes 1446 pv b1c3
info depth 5 score cp -1 time 1 nodes 5645 nps 5645000 pv b1c3
info depth 6 score cp 1 time 3 nodes 17715 nps 5905000 pv b1c3
info depth 7 score cp 7 time 10 nodes 59645 nps 5964500 pv b1c3
info depth 8 score cp 16 time 34 nodes 203957 nps 5998735 pv b1c3
info depth 9 score cp 9 time 191 nodes 1083548 nps 5673026 pv b1c3
info depth 10 score cp 13 time 648 nodes 3556915 nps 5489066 pv b1c3
info depth 11 score cp 9 time 3289 nodes 17643223 nps 5364312 pv e2e4
bestmove e2e4
17643223 nodes 5364312 nps
```

```py
Elo   | -1.47 +- 1.73 (95%)
SPRT  | 10.0+0.10s Threads=1 Hash=1MB
LLR   | -0.13 (-2.94, 2.94) [-5.00, 0.00]
Games | N: 51372 W: 18492 L: 18710 D: 14170
Penta | [1408, 3060, 16893, 2992, 1333]
https://gedas.pythonanywhere.com/test/129/
```

### 0.88

Reduce size by printing char by char

64 bit: 4064 bytes (-8)
Fill: 0

MD5: 21757ced5d4319328b5f7d6ade58225b

### 0.89

Store capture inside move struct

64 bit: 4096 bytes (+24)
Fill: 0

MD5: 488a6cc0115df9928c2ffcb7dd5bf63d

```py
info depth 1 score cp 28 time 1 nodes 20 nps 20000 pv b1c3
info depth 2 score cp 16 time 1 nodes 76 nps 76000 pv b1c3
info depth 3 score cp 26 time 1 nodes 516 nps 516000 pv b1c3
info depth 4 score cp 16 time 1 nodes 1446 nps 1446000 pv b1c3
info depth 5 score cp -1 time 1 nodes 5622 nps 5622000 pv b1c3
info depth 6 score cp 1 time 3 nodes 17490 nps 5830000 pv b1c3
info depth 7 score cp 7 time 9 nodes 59570 nps 6618888 pv b1c3
info depth 8 score cp 16 time 30 nodes 200270 nps 6675666 pv b1c3
info depth 9 score cp 9 time 170 nodes 1078818 nps 6345988 pv b1c3
info depth 10 score cp 16 time 459 nodes 2867329 nps 6246904 pv b1c3
info depth 11 score cp 12 time 2238 nodes 13720224 nps 6130573 pv b1c3
bestmove b1c3
13720224 nodes 6130573 nps
```

```py
Elo   | 47.26 +- 15.24 (95%)
SPRT  | 10.0+0.10s Threads=1 Hash=1MB
LLR   | 3.01 (-2.94, 2.94) [0.00, 5.00]
Games | N: 1324 W: 567 L: 388 D: 369
Penta | [50, 95, 249, 162, 106]
https://gedas.pythonanywhere.com/test/142/
```

```py
Score of 4k.c-0.89 vs stash12: 3492 - 1145 - 863  [0.713] 5500
...      4k.c-0.89 playing White: 1952 - 422 - 376  [0.778] 2750
...      4k.c-0.89 playing Black: 1540 - 723 - 487  [0.649] 2750
...      White vs Black: 2675 - 1962 - 863  [0.565] 5500
Elo difference: 158.4 +/- 9.1, LOS: 100.0 %, DrawRatio: 15.7 %

Score of 4k.c-0.89 vs stash13: 549 - 286 - 165  [0.631] 1000
...      4k.c-0.89 playing White: 313 - 112 - 75  [0.701] 500
...      4k.c-0.89 playing Black: 236 - 174 - 90  [0.562] 500
...      White vs Black: 487 - 348 - 165  [0.570] 1000
Elo difference: 93.6 +/- 20.2, LOS: 100.0 %, DrawRatio: 16.5 %

Score of 4k.c-0.89 vs stash14: 931 - 710 - 359  [0.555] 2000
...      4k.c-0.89 playing White: 558 - 269 - 173  [0.644] 1000
...      4k.c-0.89 playing Black: 373 - 441 - 186  [0.466] 1000
...      White vs Black: 999 - 642 - 359  [0.589] 2000
Elo difference: 38.5 +/- 13.8, LOS: 100.0 %, DrawRatio: 17.9 %

Score of 4k.c-0.89 vs stash15: 1295 - 1721 - 484  [0.439] 3500
...      4k.c-0.89 playing White: 799 - 717 - 235  [0.523] 1751
...      4k.c-0.89 playing Black: 496 - 1004 - 249  [0.355] 1749
...      White vs Black: 1803 - 1213 - 484  [0.584] 3500
Elo difference: -42.5 +/- 10.7, LOS: 0.0 %, DrawRatio: 13.8 %
```

### 0.90

Killer heuristic and size reductions to fit it

64 bit: 4096 bytes (=)
Fill: 1

```py
perft 6
info depth 6 nodes 119060324 time 3597 nps 33099895
```

```py
info depth 1 score cp 28 time 0 nodes 20 pv b1c3
info depth 2 score cp 16 time 0 nodes 76 pv b1c3
info depth 3 score cp 26 time 0 nodes 516 pv b1c3
info depth 4 score cp 16 time 0 nodes 1446 pv b1c3
info depth 5 score cp -1 time 1 nodes 5624 nps 5624000 pv b1c3
info depth 6 score cp 1 time 2 nodes 17488 nps 8744000 pv b1c3
info depth 7 score cp 7 time 8 nodes 59172 nps 7396500 pv b1c3
info depth 8 score cp 16 time 28 nodes 197277 nps 7045607 pv b1c3
info depth 9 score cp 9 time 112 nodes 750089 nps 6697223 pv b1c3
info depth 10 score cp 13 time 378 nodes 2440211 nps 6455584 pv b1c3
info depth 11 score cp 15 time 2258 nodes 13875847 nps 6145193 pv d2d4
bestmove d2d4
13875847 nodes 6145193 nps
```

```py
Elo   | 16.14 +- 8.60 (95%)
SPRT  | 10.0+0.10s Threads=1 Hash=1MB
LLR   | 3.01 (-2.94, 2.94) [0.00, 5.00]
Games | N: 4244 W: 1648 L: 1451 D: 1145
Penta | [224, 374, 794, 441, 289]
https://gedas.pythonanywhere.com/test/165/
```

```py
Score of 4k.c-0.90 vs stash14: 1039 - 621 - 341  [0.604] 2001
...      4k.c-0.90 playing White: 600 - 223 - 178  [0.688] 1001
...      4k.c-0.90 playing Black: 439 - 398 - 163  [0.520] 1000
...      White vs Black: 998 - 662 - 341  [0.584] 2001
Elo difference: 73.7 +/- 14.1, LOS: 100.0 %, DrawRatio: 17.0 %

Score of 4k.c-0.90 vs stash15: 828 - 846 - 326  [0.495] 2000
...      4k.c-0.90 playing White: 506 - 349 - 145  [0.579] 1000
...      4k.c-0.90 playing Black: 322 - 497 - 181  [0.412] 1000
...      White vs Black: 1003 - 671 - 326  [0.583] 2000
Elo difference: -3.1 +/- 13.9, LOS: 33.0 %, DrawRatio: 16.3 %

Score of 4k.c-0.90 vs stash16: 637 - 1041 - 322  [0.399] 2000
...      4k.c-0.90 playing White: 388 - 440 - 172  [0.474] 1000
...      4k.c-0.90 playing Black: 249 - 601 - 150  [0.324] 1000
...      White vs Black: 989 - 689 - 322  [0.575] 2000
Elo difference: -71.2 +/- 14.2, LOS: 0.0 %, DrawRatio: 16.1 %
```

### 0.91

Reduce size by rewriting piece movegen

64 bit: 3976 bytes (-120)
Fill: 1

```py
perft 6
info depth 6 nodes 119060324 time 3595 nps 33118309
```

```py
info depth 1 score cp 28 time 0 nodes 20 pv b1c3
info depth 2 score cp 16 time 0 nodes 76 pv b1c3
info depth 3 score cp 26 time 0 nodes 516 pv b1c3
info depth 4 score cp 16 time 0 nodes 1446 pv b1c3
info depth 5 score cp -1 time 1 nodes 5624 nps 5624000 pv b1c3
info depth 6 score cp 1 time 3 nodes 17488 nps 5829333 pv b1c3
info depth 7 score cp 7 time 9 nodes 59172 nps 6574666 pv b1c3
info depth 8 score cp 16 time 31 nodes 197276 nps 6363741 pv b1c3
info depth 9 score cp 9 time 121 nodes 749923 nps 6197710 pv b1c3
info depth 10 score cp 13 time 405 nodes 2439701 nps 6023953 pv b1c3
info depth 11 score cp 15 time 2400 nodes 13851742 nps 5771559 pv d2d4
bestmove d2d4
13851742 nodes 5771559 nps
```

### 0.92

Forward futility pruning

64 bit: 4064 bytes (+96)
Fill: 1+6

MD5: 0ba3915d062dbad43df2a1a86c399acb

```py
info depth 1 score cp 28 time 0 nodes 20 pv b1c3
info depth 2 score cp 16 time 0 nodes 76 pv b1c3
info depth 3 score cp 26 time 0 nodes 516 pv b1c3
info depth 4 score cp 16 time 0 nodes 1446 pv b1c3
info depth 5 score cp -1 time 1 nodes 5624 nps 5624000 pv b1c3
info depth 6 score cp 1 time 3 nodes 17271 nps 5757000 pv b1c3
info depth 7 score cp 7 time 9 nodes 56915 nps 6323888 pv b1c3
info depth 8 score cp 16 time 28 nodes 184437 nps 6587035 pv b1c3
info depth 9 score cp 9 time 108 nodes 679083 nps 6287805 pv b1c3
info depth 10 score cp 13 time 361 nodes 2196293 nps 6083914 pv b1c3
info depth 11 score cp 8 time 2558 nodes 14913502 nps 5830141 pv b1c3
bestmove b1c3
14913502 nodes 5830141 nps
```

```py
Elo   | 25.96 +- 11.23 (95%)
SPRT  | 10.0+0.10s Threads=1 Hash=1MB
LLR   | 3.02 (-2.94, 2.94) [0.00, 5.00]
Games | N: 2534 W: 1003 L: 814 D: 717
Penta | [124, 219, 453, 286, 185]
https://gedas.pythonanywhere.com/test/179/
```

### 0.93

Slightly faster position comparison

64 bit: 4064 bytes (=)
Fill: 1+6

MD5: 0ccb89edfee314dfb6fb9be4c134b509

```py
info depth 1 score cp 28 time 0 nodes 20 pv b1c3
info depth 2 score cp 16 time 0 nodes 76 pv b1c3
info depth 3 score cp 26 time 0 nodes 516 pv b1c3
info depth 4 score cp 16 time 0 nodes 1446 pv b1c3
info depth 5 score cp -1 time 1 nodes 5624 nps 5624000 pv b1c3
info depth 6 score cp 1 time 3 nodes 17271 nps 5757000 pv b1c3
info depth 7 score cp 7 time 10 nodes 56915 nps 5691500 pv b1c3
info depth 8 score cp 16 time 33 nodes 184437 nps 5589000 pv b1c3
info depth 9 score cp 9 time 128 nodes 679083 nps 5305335 pv b1c3
info depth 10 score cp 13 time 424 nodes 2196293 nps 5179936 pv b1c3
info depth 11 score cp 8 time 3031 nodes 14913502 nps 4920323 pv b1c3
bestmove b1c3
14913502 nodes 4920323 nps
```

```py
Elo   | 9.13 +- 7.01 (95%)
SPRT  | 10.0+0.10s Threads=1 Hash=1MB
LLR   | 3.01 (-2.94, 2.94) [-5.00, 0.00]
Games | N: 4722 W: 1730 L: 1606 D: 1386
Penta | [165, 410, 1135, 438, 213]
https://gedas.pythonanywhere.com/test/185/
```

### 0.94

Use promotion material in forward futility pruning

64 bit: 4080 bytes (+16)
Fill: 1+6

MD5: 39de294a43458f4bf68989acbc220c85

```py
info depth 1 score cp 28 time 0 nodes 20 pv b1c3
info depth 2 score cp 16 time 0 nodes 76 pv b1c3
info depth 3 score cp 26 time 1 nodes 516 nps 516000 pv b1c3
info depth 4 score cp 16 time 1 nodes 1446 nps 1446000 pv b1c3
info depth 5 score cp -1 time 1 nodes 5624 nps 5624000 pv b1c3
info depth 6 score cp 1 time 3 nodes 17271 nps 5757000 pv b1c3
info depth 7 score cp 7 time 9 nodes 56915 nps 6323888 pv b1c3
info depth 8 score cp 16 time 27 nodes 184437 nps 6831000 pv b1c3
info depth 9 score cp 9 time 105 nodes 679083 nps 6467457 pv b1c3
info depth 10 score cp 13 time 351 nodes 2196293 nps 6257245 pv b1c3
info depth 11 score cp 8 time 2505 nodes 14913502 nps 5953493 pv b1c3
bestmove b1c3
14913502 nodes 5953493 nps
```

```py
Elo   | 7.90 +- 5.26 (95%)
SPRT  | 10.0+0.10s Threads=1 Hash=1MB
LLR   | 3.01 (-2.94, 2.94) [0.00, 5.00]
Games | N: 8398 W: 2896 L: 2705 D: 2797
Penta | [312, 729, 1957, 858, 343]
https://gedas.pythonanywhere.com/test/180/
```

### 0.95

Reorder castling in movegen

64 bit: 4072 bytes (-8)
Fill: 4+6

MD5: 4677ae9d3a62d9c8bbf3ce6ab744b8ca

```py
info depth 1 score cp 28 time 0 nodes 20 pv b1c3
info depth 2 score cp 16 time 0 nodes 76 pv b1c3
info depth 3 score cp 26 time 0 nodes 516 pv b1c3
info depth 4 score cp 16 time 0 nodes 1446 pv b1c3
info depth 5 score cp -1 time 1 nodes 5624 nps 5624000 pv b1c3
info depth 6 score cp 1 time 3 nodes 17271 nps 5757000 pv b1c3
info depth 7 score cp 7 time 9 nodes 56915 nps 6323888 pv b1c3
info depth 8 score cp 16 time 28 nodes 184437 nps 6587035 pv b1c3
info depth 9 score cp 9 time 109 nodes 679064 nps 6229944 pv b1c3
info depth 10 score cp 13 time 364 nodes 2196182 nps 6033467 pv b1c3
info depth 11 score cp 8 time 2558 nodes 14956082 nps 5846787 pv b1c3
bestmove b1c3
14956082 nodes 5846787 nps
```

```py
Elo   | 4.21 +- 5.57 (95%)
SPRT  | 10.0+0.10s Threads=1 Hash=1MB
LLR   | 2.95 (-2.94, 2.94) [-5.00, 0.00]
Games | N: 8992 W: 3264 L: 3155 D: 2573
Penta | [432, 868, 1838, 875, 483]
https://gedas.pythonanywhere.com/test/192/
```

### 0.96

Allow castling in qsearch

64 bit: 4064 bytes (-8)
Fill: 4+6

MD5: 866e9de7020cb42d2235fdbc34f10d75

```py
info depth 1 score cp 28 time 0 nodes 20 pv b1c3
info depth 2 score cp 16 time 0 nodes 76 pv b1c3
info depth 3 score cp 26 time 0 nodes 516 pv b1c3
info depth 4 score cp 16 time 1 nodes 1446 nps 1446000 pv b1c3
info depth 5 score cp -1 time 1 nodes 5624 nps 5624000 pv b1c3
info depth 6 score cp 1 time 3 nodes 17271 nps 5757000 pv b1c3
info depth 7 score cp 7 time 8 nodes 56915 nps 7114375 pv b1c3
info depth 8 score cp 16 time 27 nodes 184539 nps 6834777 pv b1c3
info depth 9 score cp 9 time 106 nodes 682183 nps 6435688 pv b1c3
info depth 10 score cp 13 time 356 nodes 2215319 nps 6222806 pv b1c3
info depth 11 score cp 8 time 2535 nodes 15146501 nps 5974951 pv b1c3
bestmove b1c3
15146501 nodes 5974951 nps
```

```py
Elo   | 4.32 +- 4.99 (95%)
SPRT  | 10.0+0.10s Threads=1 Hash=1MB
LLR   | 3.01 (-2.94, 2.94) [-5.00, 0.00]
Games | N: 7954 W: 2844 L: 2745 D: 2365
Penta | [246, 622, 2182, 641, 286]
https://gedas.pythonanywhere.com/test/201/
```

### 0.97

No repetition detection in qsearch

64 bit: 4080 bytes (+16)
Fill: 6

MD5: 9a477d93b47fa7478a4915e0a2b58c10

```py
info depth 1 score cp 28 time 0 nodes 20 pv b1c3
info depth 2 score cp 16 time 0 nodes 76 pv b1c3
info depth 3 score cp 26 time 0 nodes 516 pv b1c3
info depth 4 score cp 16 time 0 nodes 1446 pv b1c3
info depth 5 score cp -1 time 1 nodes 5624 nps 5624000 pv b1c3
info depth 6 score cp 1 time 2 nodes 17271 nps 8635500 pv b1c3
info depth 7 score cp 7 time 8 nodes 56915 nps 7114375 pv b1c3
info depth 8 score cp 16 time 25 nodes 184539 nps 7381560 pv b1c3
info depth 9 score cp 9 time 99 nodes 682183 nps 6890737 pv b1c3
info depth 10 score cp 13 time 331 nodes 2213722 nps 6687981 pv b1c3
info depth 11 score cp 14 time 2220 nodes 14133336 nps 6366367 pv d2d4
bestmove d2d4
14133336 nodes 6366367 nps
```

```py
Elo   | 15.67 +- 8.47 (95%)
SPRT  | 10.0+0.10s Threads=1 Hash=1MB
LLR   | 2.90 (-2.94, 2.94) [0.00, 5.00]
Games | N: 4038 W: 1538 L: 1356 D: 1144
Penta | [189, 352, 817, 410, 251]
https://gedas.pythonanywhere.com/test/213/
```

```py
Score of 4k.c-0.97 vs stash15: 437 - 376 - 187  [0.530] 1000
...      4k.c-0.97 playing White: 271 - 132 - 97  [0.639] 500
...      4k.c-0.97 playing Black: 166 - 244 - 90  [0.422] 500
...      White vs Black: 515 - 298 - 187  [0.609] 1000
Elo difference: 21.2 +/- 19.4, LOS: 98.4 %, DrawRatio: 18.7 %

Score of 4k.c-0.97 vs stash16: 352 - 489 - 159  [0.431] 1000
...      4k.c-0.97 playing White: 219 - 195 - 86  [0.524] 500
...      4k.c-0.97 playing Black: 133 - 294 - 73  [0.339] 500
...      White vs Black: 513 - 328 - 159  [0.593] 1000
Elo difference: -47.9 +/- 19.9, LOS: 0.0 %, DrawRatio: 15.9 %
```

### 0.98

Pawn double-moves first

64 bit: 4064 bytes (-16)
Fill: 1+6

MD5: 541add2730303f5295dad30d710feab2

```py
info depth 1 score cp 28 time 1 nodes 20 nps 20000 pv b1c3
info depth 2 score cp 16 time 1 nodes 76 nps 76000 pv b1c3
info depth 3 score cp 26 time 1 nodes 514 nps 514000 pv b1c3
info depth 4 score cp 16 time 1 nodes 1442 nps 1442000 pv b1c3
info depth 5 score cp -1 time 1 nodes 5587 nps 5587000 pv b1c3
info depth 6 score cp 1 time 3 nodes 17505 nps 5835000 pv b1c3
info depth 7 score cp 7 time 8 nodes 53760 nps 6720000 pv b1c3
info depth 8 score cp 16 time 28 nodes 186557 nps 6662750 pv b1c3
info depth 9 score cp 7 time 122 nodes 793284 nps 6502327 pv b1c3
info depth 10 score cp 13 time 520 nodes 3297205 nps 6340778 pv g1f3
info depth 11 score cp 11 time 2624 nodes 15889443 nps 6055427 pv b1c3
bestmove b1c3
15889443 nodes 6055427 nps
```

```py
Elo   | -2.16 +- 2.05 (95%)
SPRT  | 10.0+0.10s Threads=1 Hash=1MB
LLR   | -0.57 (-2.94, 2.94) [-5.00, 0.00]
Games | N: 71168 W: 24953 L: 25396 D: 20819
Penta | [3981, 7360, 13218, 7171, 3854]
https://gedas.pythonanywhere.com/test/221/
```

### 0.99

Bishop pair evaluation

64 bit: 4088 bytes (+24)
Fill: 2+6

MD5: 4fe6511f871edba5485a4169310e93dd

```py
info depth 1 score cp 28 time 0 nodes 20 pv b1c3
info depth 2 score cp 16 time 0 nodes 76 pv b1c3
info depth 3 score cp 26 time 0 nodes 514 pv b1c3
info depth 4 score cp 16 time 0 nodes 1442 pv b1c3
info depth 5 score cp 0 time 1 nodes 5509 nps 5509000 pv b1c3
info depth 6 score cp 16 time 3 nodes 16815 nps 5605000 pv b1c3
info depth 7 score cp 7 time 8 nodes 54682 nps 6835250 pv b1c3
info depth 8 score cp 16 time 27 nodes 183693 nps 6803444 pv b1c3
info depth 9 score cp 8 time 95 nodes 642517 nps 6763336 pv b1c3
info depth 10 score cp 9 time 413 nodes 2671327 nps 6468104 pv b1c3
info depth 11 score cp 12 time 2337 nodes 14434289 nps 6176418 pv b1c3
bestmove b1c3
14434289 nodes 6176418 nps
```

```py
Elo   | 9.14 +- 6.07 (95%)
SPRT  | 10.0+0.10s Threads=1 Hash=1MB
LLR   | 2.97 (-2.94, 2.94) [0.00, 5.00]
Games | N: 8024 W: 2852 L: 2641 D: 2531
Penta | [390, 826, 1432, 911, 453]
https://gedas.pythonanywhere.com/test/227/
```

```py
Score of 4k.c-0.99 vs stash15: 477 - 378 - 145  [0.549] 1000
...      4k.c-0.99 playing White: 281 - 145 - 74  [0.636] 500
...      4k.c-0.99 playing Black: 196 - 233 - 71  [0.463] 500
...      White vs Black: 514 - 341 - 145  [0.587] 1000
Elo difference: 34.5 +/- 20.0, LOS: 100.0 %, DrawRatio: 14.5 %
```

### 0.100

Smaller forward futility pruning

64 bit: 4072 bytes (-16)
Fill: 6+6

MD5: ee6667f63ad57feda7bcba60d59e5261

```py
info depth 1 score cp 28 time 0 nodes 20 pv b1c3
info depth 2 score cp 16 time 0 nodes 76 pv b1c3
info depth 3 score cp 26 time 0 nodes 514 pv b1c3
info depth 4 score cp 16 time 1 nodes 1442 nps 1442000 pv b1c3
info depth 5 score cp 0 time 1 nodes 5509 nps 5509000 pv b1c3
info depth 6 score cp 16 time 2 nodes 16815 nps 8407500 pv b1c3
info depth 7 score cp 7 time 7 nodes 54682 nps 7811714 pv b1c3
info depth 8 score cp 16 time 25 nodes 183693 nps 7347720 pv b1c3
info depth 9 score cp 8 time 89 nodes 642517 nps 7219292 pv b1c3
info depth 10 score cp 9 time 390 nodes 2671327 nps 6849556 pv b1c3
info depth 11 score cp 12 time 2238 nodes 14434289 nps 6449637 pv b1c3
bestmove b1c3
14434289 nodes 6449637 nps
```

```py
Elo   | 1.23 +- 3.63 (95%)
SPRT  | 10.0+0.10s Threads=1 Hash=1MB
LLR   | 3.00 (-2.94, 2.94) [-5.00, 0.00]
Games | N: 16986 W: 6093 L: 6033 D: 4860
Penta | [668, 1374, 4363, 1406, 682]
https://gedas.pythonanywhere.com/test/281/
```

### 0.101

64 bit: 4064 bytes (-8)
Fill: 5+6

MD5: 4b16f2f7f3afd2cc13166b9cd7d932ab

No ply check for early exits

```py
info depth 1 score cp 28 time 0 nodes 20 pv b1c3
info depth 2 score cp 16 time 0 nodes 76 pv b1c3
info depth 3 score cp 26 time 0 nodes 514 pv b1c3
info depth 4 score cp 16 time 0 nodes 1442 pv b1c3
info depth 5 score cp 0 time 1 nodes 5509 nps 5509000 pv b1c3
info depth 6 score cp 16 time 2 nodes 16815 nps 8407500 pv b1c3
info depth 7 score cp 7 time 8 nodes 54682 nps 6835250 pv b1c3
info depth 8 score cp 16 time 26 nodes 183693 nps 7065115 pv b1c3
info depth 9 score cp 8 time 93 nodes 642517 nps 6908784 pv b1c3
info depth 10 score cp 9 time 404 nodes 2671327 nps 6612195 pv b1c3
info depth 11 score cp 12 time 2329 nodes 14434289 nps 6197633 pv b1c3
bestmove b1c3
14434289 nodes 6197633 nps
```

```py
Elo   | 0.89 +- 3.54 (95%)
SPRT  | 10.0+0.10s Threads=1 Hash=1MB
LLR   | 3.01 (-2.94, 2.94) [-5.00, 0.00]
Games | N: 19532 W: 7037 L: 6987 D: 5508
Penta | [863, 1689, 4603, 1757, 854]
https://gedas.pythonanywhere.com/test/283/
```

### 0.102

64 bit: 4080 bytes (+16)
Fill: 6

MD5: 50f9262d999f4878b0d0d1875984035a

Reset history on capture

```py
Elo   | 8.95 +- 5.63 (95%)
SPRT  | 10.0+0.10s Threads=1 Hash=1MB
LLR   | 2.98 (-2.94, 2.94) [0.00, 5.00]
Games | N: 6678 W: 2452 L: 2280 D: 1946
Penta | [203, 553, 1726, 583, 274]
https://gedas.pythonanywhere.com/test/298/
```

```py
Score of 4k.c-0.102 vs stash15: 931 - 742 - 327  [0.547] 2000
...      4k.c-0.102 playing White: 567 - 290 - 142  [0.639] 999
...      4k.c-0.102 playing Black: 364 - 452 - 185  [0.456] 1001
...      White vs Black: 1019 - 654 - 327  [0.591] 2000
Elo difference: 32.9 +/- 14.0, LOS: 100.0 %, DrawRatio: 16.4 %
```

```py
Score of 4k.c-0.102 vs stash16: 837 - 1122 - 341  [0.438] 2300
...      4k.c-0.102 playing White: 528 - 444 - 179  [0.536] 1151
...      4k.c-0.102 playing Black: 309 - 678 - 162  [0.339] 1149
...      White vs Black: 1206 - 753 - 341  [0.598] 2300
Elo difference: -43.3 +/- 13.2, LOS: 0.0 %, DrawRatio: 14.8 %
```

### 0.103

64 bit: 4072 bytes (-8)
Fill: 1+6

MD5: 9baf56adc83247f7e8a9594832ebce5f

Make mate detection branchless

```py
Elo   | -0.72 +- 2.94 (95%)
SPRT  | 10.0+0.10s Threads=1 Hash=1MB
LLR   | 1.30 (-2.94, 2.94) [-5.00, 0.00]
Games | N: 22624 W: 7571 L: 7618 D: 7435
Penta | [714, 1922, 6086, 1877, 713]
https://gedas.pythonanywhere.com/test/301/
```

### 0.104

64 bit: 4072 bytes (=)
Fill: 4+6

MD5: 8a5543c033890e9e1e16b416cc24c147

Reverse futility pruning margin 48

```py
info depth 1 score cp 28 time 0 nodes 20 pv b1c3
info depth 2 score cp 16 time 1 nodes 76 nps 76000 pv b1c3
info depth 3 score cp 26 time 1 nodes 514 nps 514000 pv b1c3
info depth 4 score cp 16 time 1 nodes 1442 nps 1442000 pv b1c3
info depth 5 score cp 0 time 1 nodes 5362 nps 5362000 pv b1c3
info depth 6 score cp 16 time 3 nodes 16626 nps 5542000 pv b1c3
info depth 7 score cp 7 time 7 nodes 43445 nps 6206428 pv b1c3
info depth 8 score cp 16 time 23 nodes 146219 nps 6357347 pv b1c3
info depth 9 score cp 8 time 78 nodes 500792 nps 6420410 pv b1c3
info depth 10 score cp 16 time 268 nodes 1666285 nps 6217481 pv b1c3
info depth 11 score cp 16 time 994 nodes 6054500 nps 6091046 pv b1c3
info depth 12 score cp 13 time 4707 nodes 27934527 nps 5934677 pv b1c3
bestmove b1c3
27934527 nodes 5934677 nps
```

```py
Elo   | 7.35 +- 5.22 (95%)
SPRT  | 10.0+0.10s Threads=1 Hash=1MB
LLR   | 3.04 (-2.94, 2.94) [0.00, 5.00]
Games | N: 11162 W: 4079 L: 3843 D: 3240
Penta | [591, 1084, 2081, 1148, 677]
https://gedas.pythonanywhere.com/test/288/
```

```py
Elo   | 3.04 +- 3.98 (95%)
SPRT  | 60.0+0.60s Threads=1 Hash=1MB
LLR   | 1.04 (-2.94, 2.94) [0.00, 5.00]
Games | N: 17480 W: 5982 L: 5829 D: 5669
Penta | [822, 1815, 3353, 1888, 862]
https://gedas.pythonanywhere.com/test/290/
```

### 0.105

64 bit: 4064 bytes (-8)
Fill: 2+6

MD5: 08967df5e61221b3a9410898de9690b8

Smaller moves evaluated

### 0.106

64 bit: 4096 bytes (+32)
Fill: 7+6

MD5: 520c158f4c837eb6b813ea72e6c5eca2

Doubled pawn evaluation

```py
info depth 1 score cp 29 time 0 nodes 20 pv b1c3
info depth 2 score cp 16 time 0 nodes 76 pv b1c3
info depth 3 score cp 27 time 0 nodes 514 pv b1c3
info depth 4 score cp 16 time 0 nodes 1443 pv b1c3
info depth 5 score cp 0 time 1 nodes 6130 nps 6130000 pv b1c3
info depth 6 score cp 16 time 2 nodes 17445 nps 8722500 pv b1c3
info depth 7 score cp 8 time 7 nodes 48263 nps 6894714 pv b1c3
info depth 8 score cp 16 time 23 nodes 149877 nps 6516391 pv b1c3
info depth 9 score cp 8 time 110 nodes 708933 nps 6444845 pv e2e4
info depth 10 score cp 16 time 388 nodes 2493586 nps 6426768 pv g1f3
info depth 11 score cp 12 time 1194 nodes 7565504 nps 6336268 pv g1f3
info depth 12 score cp 9 time 4183 nodes 25892201 nps 6189863 pv g1f3
bestmove g1f3
25892201 nodes 6188384 nps
```

```py
Elo   | 19.81 +- 9.48 (95%)
SPRT  | 10.0+0.10s Threads=1 Hash=1MB
LLR   | 3.11 (-2.94, 2.94) [0.00, 5.00]
Games | N: 3442 W: 1339 L: 1143 D: 960
Penta | [151, 346, 629, 346, 249]
https://gedas.pythonanywhere.com/test/311/
```

```py
Score of 4k.c-0.106 vs stash15: 472 - 363 - 166  [0.554] 1001
...      4k.c-0.106 playing White: 270 - 157 - 74  [0.613] 501
...      4k.c-0.106 playing Black: 202 - 206 - 92  [0.496] 500
...      White vs Black: 476 - 359 - 166  [0.558] 1001
Elo difference: 38.0 +/- 19.7, LOS: 100.0 %, DrawRatio: 16.6 %

Score of 4k.c-0.106 vs stash16: 758 - 937 - 305  [0.455] 2000
...      4k.c-0.106 playing White: 481 - 375 - 144  [0.553] 1000
...      4k.c-0.106 playing Black: 277 - 562 - 161  [0.357] 1000
...      White vs Black: 1043 - 652 - 305  [0.598] 2000
Elo difference: -31.2 +/- 14.1, LOS: 0.0 %, DrawRatio: 15.3 %
```

### 0.107

64 bit: 4096 bytes (=)
Fill: 6

MD5: 3a0f8572f63294014ed05186e57c957a

Allow FFP in QS

```py
info depth 1 score cp 29 time 0 nodes 20 pv b1c3
info depth 2 score cp 16 time 1 nodes 75 nps 75000 pv b1c3
info depth 3 score cp 27 time 1 nodes 493 nps 493000 pv b1c3
info depth 4 score cp 16 time 1 nodes 1401 nps 1401000 pv b1c3
info depth 5 score cp 0 time 1 nodes 5874 nps 5874000 pv b1c3
info depth 6 score cp 16 time 3 nodes 16472 nps 5490666 pv b1c3
info depth 7 score cp 8 time 7 nodes 45508 nps 6501142 pv b1c3
info depth 8 score cp 16 time 24 nodes 157408 nps 6558666 pv b1c3
info depth 9 score cp 6 time 78 nodes 508892 nps 6524256 pv b1c3
info depth 10 score cp 16 time 245 nodes 1564690 nps 6386489 pv b1c3
info depth 11 score cp 10 time 1022 nodes 6323987 nps 6187854 pv b1c3
info depth 12 score cp 15 time 4528 nodes 26860197 nps 5932022 pv g1f3
bestmove g1f3
26860197 nodes 5932022 nps
```

```py
Elo   | 7.28 +- 5.26 (95%)
SPRT  | 10.0+0.10s Threads=1 Hash=1MB
LLR   | 2.95 (-2.94, 2.94) [0.00, 5.00]
Games | N: 10880 W: 4037 L: 3809 D: 3034
Penta | [578, 1031, 2061, 1125, 645]
https://gedas.pythonanywhere.com/test/326/
```

```py
Elo   | 3.18 +- 4.29 (95%)
SPRT  | 60.0+0.60s Threads=1 Hash=1MB
LLR   | 1.01 (-2.94, 2.94) [0.00, 5.00]
Games | N: 15280 W: 5259 L: 5119 D: 4902
Penta | [700, 1684, 2803, 1682, 771]
https://gedas.pythonanywhere.com/test/328/
```

```py
Score of 4k.c-0.107 vs stash15: 965 - 736 - 299  [0.557] 2000
...      4k.c-0.107 playing White: 568 - 298 - 133  [0.635] 999
...      4k.c-0.107 playing Black: 397 - 438 - 166  [0.480] 1001
...      White vs Black: 1006 - 695 - 299  [0.578] 2000
Elo difference: 40.0 +/- 14.1, LOS: 100.0 %, DrawRatio: 14.9 %
```

### 0.108

64 bit: 4096 bytes (=)
Fill: 5+6

MD5: 5edc18c5ad92e7debfd9ec9d222cc5aa

Smaller mobility

```py
Elo   | 4.37 +- 4.91 (95%)
SPRT  | 10.0+0.10s Threads=1 Hash=1MB
LLR   | 2.97 (-2.94, 2.94) [-5.00, 0.00]
Games | N: 7548 W: 2716 L: 2621 D: 2211
Penta | [217, 527, 2226, 552, 252]
https://gedas.pythonanywhere.com/test/348/
```

### 0.109

64 bit: 4088 bytes (-8)
Fill: 2+6

MD5: be06d3d881e6313d8ae7326cd04cdec0

Smaller pawn movegen

```py
Elo   | 0.02 +- 2.39 (95%)
SPRT  | 10.0+0.10s Threads=1 Hash=1MB
LLR   | 2.95 (-2.94, 2.94) [-5.00, 0.00]
Games | N: 28018 W: 9970 L: 9968 D: 8080
Penta | [756, 1781, 8949, 1751, 772]
https://gedas.pythonanywhere.com/test/345/
```

### 0.110

64 bit: 4080 bytes (-8)
Fill: 1+6

MD5: 178cc5ab64dfd1e91ec48caf2e3c5060

Use current master branch of GCC for build

```py
Score of 4k.c-dev vs 4k.c-0.109: 691 - 627 - 682  [0.516] 2000
...      4k.c-dev playing White: 421 - 239 - 341  [0.591] 1001
...      4k.c-dev playing Black: 270 - 388 - 341  [0.441] 999
...      White vs Black: 809 - 509 - 682  [0.575] 2000
Elo difference: 11.1 +/- 12.4, LOS: 96.1 %, DrawRatio: 34.1 %
Started game 2018 of 200000 (4k.c-0.109 vs 4k.c-dev)
```

### 0.111

64 bit: 4096 bytes (+16)
Fill: 7+6

MD5: 500c5efd99ad1f46acfd46b8128e1d21

Extended doubled pawn eval

```py
info depth 1 score cp 28 time 0 nodes 20 pv b1c3
info depth 2 score cp 16 time 0 nodes 75 pv b1c3
info depth 3 score cp 26 time 1 nodes 493 nps 493000 pv b1c3
info depth 4 score cp 16 time 1 nodes 1401 nps 1401000 pv b1c3
info depth 5 score cp 0 time 1 nodes 5931 nps 5931000 pv b1c3
info depth 6 score cp 16 time 3 nodes 16532 nps 5510666 pv b1c3
info depth 7 score cp 8 time 7 nodes 45706 nps 6529428 pv b1c3
info depth 8 score cp 16 time 24 nodes 158600 nps 6608333 pv b1c3
info depth 9 score cp 9 time 76 nodes 490736 nps 6457052 pv b1c3
info depth 10 score cp 14 time 269 nodes 1673529 nps 6221297 pv b1c3
info depth 11 score cp 14 time 879 nodes 5303794 nps 6033895 pv b1c3
info depth 12 score cp 12 time 3835 nodes 22196884 nps 5787974 pv d2d4
bestmove d2d4
22196884 nodes 5787974 nps
```

```py
Elo   | 3.03 +- 2.28 (95%)
SPRT  | 10.0+0.10s Threads=1 Hash=1MB
LLR   | 3.00 (-2.94, 2.94) [0.00, 5.00]
Games | N: 58762 W: 21377 L: 20864 D: 16521
Penta | [3303, 5776, 10886, 5937, 3479]
https://gedas.pythonanywhere.com/test/350/
```

```py
Elo   | 5.57 +- 8.07 (95%)
SPRT  | 60.0+0.60s Threads=1 Hash=1MB
LLR   | 0.84 (-2.94, 2.94) [0.00, 5.00]
Games | N: 4550 W: 1599 L: 1526 D: 1425
Penta | [247, 427, 853, 502, 246]
https://gedas.pythonanywhere.com/test/351/
```

```py
Score of 4k.c-0.111 vs stash15: 2438 - 1810 - 752  [0.563] 5000
...      4k.c-0.111 playing White: 1409 - 763 - 329  [0.629] 2501
...      4k.c-0.111 playing Black: 1029 - 1047 - 423  [0.496] 2499
...      White vs Black: 2456 - 1792 - 752  [0.566] 5000
Elo difference: 43.9 +/- 8.9, LOS: 100.0 %, DrawRatio: 15.0 %
```

### 0.112

64 bit: 4088 bytes (-8)
Fill: 2+6

MD5: 6d756d2fa5902ca32224f634293631e0

Smaller piece takes

### 0.113

64 bit: 4088 bytes (=)
Fill: 7+6

MD5: efee3c92cc4cb1f11dc752e06989b3ae

More aggressive TM

```py
Elo   | 17.66 +- 8.85 (95%)
SPRT  | 10.0+0.10s Threads=1 Hash=1MB
LLR   | 2.95 (-2.94, 2.94) [0.00, 5.00]
Games | N: 3466 W: 1286 L: 1110 D: 1070
Penta | [136, 324, 695, 384, 194]
https://gedas.pythonanywhere.com/test/368/
```

With adjudication:
```py
Elo   | 17.31 +- 8.67 (95%)
SPRT  | 60.0+0.60s Threads=1 Hash=1MB
LLR   | 2.96 (-2.94, 2.94) [0.00, 5.00]
Games | N: 3476 W: 1253 L: 1080 D: 1143
Penta | [127, 331, 702, 398, 180]
https://gedas.pythonanywhere.com/test/366/
```

Without adjudication:
```py
Elo   | 9.79 +- 6.19 (95%)
SPRT  | 60.0+0.60s Threads=1 Hash=1MB
LLR   | 2.96 (-2.94, 2.94) [0.00, 5.00]
Games | N: 6742 W: 2281 L: 2091 D: 2370
Penta | [265, 680, 1333, 786, 307]
https://gedas.pythonanywhere.com/test/367/
```

```py
Score of 4k.c-0.113 vs stash15: 3409 - 2553 - 1038  [0.561] 7000
...      4k.c-0.113 playing White: 2005 - 1010 - 485  [0.642] 3500
...      4k.c-0.113 playing Black: 1404 - 1543 - 553  [0.480] 3500
...      White vs Black: 3548 - 2414 - 1038  [0.581] 7000
Elo difference: 42.7 +/- 7.5, LOS: 100.0 %, DrawRatio: 14.8 %
```

```py
Score of 4k.c-0.113 vs stash16: 1962 - 2304 - 734  [0.466] 5000
...      4k.c-0.113 playing White: 1192 - 939 - 369  [0.551] 2500
...      4k.c-0.113 playing Black: 770 - 1365 - 365  [0.381] 2500
...      White vs Black: 2557 - 1709 - 734  [0.585] 5000
Elo difference: -23.8 +/- 8.9, LOS: 0.0 %, DrawRatio: 14.7 %
```

No losses on time observed

### 0.114

64 bit: 4096 bytes (+8)
Fill: 2+6

MD5: 555b5c3a4288257c0b687325eb71d67b

Open files evaluation, deleting bishop pair and doubled pawns

```py
Elo   | 24.17 +- 10.85 (95%)
SPRT  | 10.0+0.10s Threads=1 Hash=1MB
LLR   | 3.04 (-2.94, 2.94) [0.00, 5.00]
Games | N: 2808 W: 1127 L: 932 D: 749
Penta | [147, 242, 502, 295, 218]
https://gedas.pythonanywhere.com/test/372/
```

```py
Score of 4k.c-0.114 vs stash16: 2110 - 2073 - 817  [0.504] 5000
...      4k.c-0.114 playing White: 1267 - 817 - 415  [0.590] 2499
...      4k.c-0.114 playing Black: 843 - 1256 - 402  [0.417] 2501
...      White vs Black: 2523 - 1660 - 817  [0.586] 5000
Elo difference: 2.6 +/- 8.8, LOS: 71.6 %, DrawRatio: 16.3 %
```

### 0.115

64 bit: 4096 bytes (=)
Fill: 1+6

MD5: 9bf8206f28d307d02b72661779288915

Branchless open files

```py
Elo   | 4.61 +- 3.49 (95%)
SPRT  | 10.0+0.10s Threads=1 Hash=1MB
LLR   | 2.96 (-2.94, 2.94) [0.00, 5.00]
Games | N: 14018 W: 5088 L: 4902 D: 4028
Penta | [370, 947, 4248, 1015, 429]
https://gedas.pythonanywhere.com/test/376/
```

### 0.116

64 bit: 4088 bytes (-8)
Fill: 1+6

MD5: 76271d249a3baabbdf5aad4359ec5d4f

Optimized move string comparison (Suggested by Zamfofex)

### 0.117

64 bit: 4080 bytes (-8)
Fill: 6

MD5: af39bfc83d5a7ea91c6f789f5c56fec3

Smaller promotions in makemove (Suggested by Zamfofex)

```py
Elo   | -0.08 +- 2.26 (95%)
SPRT  | 10.0+0.10s Threads=1 Hash=1MB
LLR   | 2.94 (-2.94, 2.94) [-5.00, 0.00]
Games | N: 30122 W: 10663 L: 10670 D: 8789
Penta | [777, 1878, 9750, 1887, 769]
https://gedas.pythonanywhere.com/test/383/
```

### 0.118

64 bit: 4072 bytes (-8)
Fill: 3+6

MD5: 33ac757e48c2235d7c567c1410a0f233

Even smaller move string comparison (Suggested by swedishchef)

### 0.119

64 bit: 4088 bytes (+16)
Fill: 6+6

MD5: 5cdd8e1a76d55256ba52ed7a13d81c6c

Proper quit command support

### 0.120

64 bit: 4088 bytes (=)
Fill: 6+6

MD5: c43978d052d157bbf5762411f87df09f

Increase line buffer for safety

### 0.121

64 bit: 4058 bytes (-30)
Fill: 6+8

MD5: 464fe849dfebae91bc425385724c8dcc

Don't include bss in binary

### 0.122

64 bit: 4018 bytes (-40)
Fill: 0

MD5: a234a9e247d7fbe6701f4a6c7b0cfffc

Remove bss prevention hack

### 0.123

64 bit: 4074 bytes (+56)
Fill: 3

MD5: a2a9678bd0d37ca76e6c43e453fef11d

Protected pawn eval

```py
info depth 1 score cp 29 time 0 nodes 20 pv b1c3
info depth 2 score cp 16 time 0 nodes 75 pv b1c3
info depth 3 score cp 26 time 0 nodes 493 pv b1c3
info depth 4 score cp 16 time 1 nodes 1398 nps 1398000 pv b1c3
info depth 5 score cp 0 time 1 nodes 5962 nps 5962000 pv b1c3
info depth 6 score cp 7 time 3 nodes 16155 nps 5385000 pv b1c3
info depth 7 score cp 10 time 8 nodes 46904 nps 5863000 pv b1c3
info depth 8 score cp 16 time 25 nodes 141120 nps 5644800 pv b1c3
info depth 9 score cp 10 time 88 nodes 509813 nps 5793329 pv b1c3
info depth 10 score cp 16 time 319 nodes 1799165 nps 5640015 pv b1c3
info depth 11 score cp 11 time 1601 nodes 8961482 nps 5597427 pv e2e3
info depth 12 score cp 11 time 4020 nodes 22525797 nps 5603432 pv e2e3
bestmove e2e3
22525797 nodes 5603432 nps
```

```py
Elo   | 14.78 +- 8.22 (95%)
SPRT  | 10.0+0.10s Threads=1 Hash=1MB
LLR   | 2.99 (-2.94, 2.94) [0.00, 5.00]
Games | N: 4704 W: 1807 L: 1607 D: 1290
Penta | [250, 442, 834, 510, 316]
https://gedas.pythonanywhere.com/test/403/
```

### 0.124

64 bit: 4042 bytes (-32)
Fill: 1

MD5: b3fe1f5bf4e21fe6662fb368fdeebf90

Shrink movegen (Implemented by cj5716)

### 0.125

64 bit: 4067 bytes (+25)

MD5: 436d35a71e0adbbe6b7b0a4486dd1597

Bishop pair eval

```py
info depth 1 score cp 31 time 0 nodes 20 pv b1c3
info depth 2 score cp 16 time 0 nodes 75 pv b1c3
info depth 3 score cp 29 time 1 nodes 491 nps 491000 pv b1c3
info depth 4 score cp 16 time 1 nodes 1396 nps 1396000 pv b1c3
info depth 5 score cp -3 time 2 nodes 5950 nps 2975000 pv b1c3
info depth 6 score cp 6 time 3 nodes 15514 nps 5171333 pv b1c3
info depth 7 score cp 13 time 9 nodes 49564 nps 5507111 pv b1c3
info depth 8 score cp 16 time 24 nodes 136461 nps 5685875 pv b1c3
info depth 9 score cp 7 time 147 nodes 814298 nps 5539442 pv g1f3
info depth 10 score cp 13 time 422 nodes 2304618 nps 5461180 pv g1f3
info depth 11 score cp 11 time 1413 nodes 7645596 nps 5410895 pv b1c3
info depth 12 score cp 10 time 6006 nodes 31023053 nps 5165343 pv e2e3
bestmove e2e3
31023053 nodes 5165343 nps
```

```py
Elo   | 9.42 +- 6.26 (95%)
SPRT  | 10.0+0.10s Threads=1 Hash=1MB
LLR   | 2.96 (-2.94, 2.94) [0.00, 5.00]
Games | N: 7932 W: 2982 L: 2767 D: 2183
Penta | [430, 743, 1489, 790, 514]
https://gedas.pythonanywhere.com/test/405/
```

### 1.0

Safety features

64 bit: 4080 bytes (+13)

60+0.6:
```py
Score of 4k.c-1.0 vs stash16: 1408 - 1897 - 695  [0.439] 4000
...      4k.c-1.0 playing White: 907 - 723 - 370  [0.546] 2000
...      4k.c-1.0 playing Black: 501 - 1174 - 325  [0.332] 2000
...      White vs Black: 2081 - 1224 - 695  [0.607] 4000
Elo difference: -42.7 +/- 9.8, LOS: 0.0 %, DrawRatio: 17.4 %
```

60+0.6:
```py
Score of 4k.c-1.0 vs stash17: 771 - 1348 - 381  [0.385] 2500
...      4k.c-1.0 playing White: 515 - 532 - 203  [0.493] 1250
...      4k.c-1.0 playing Black: 256 - 816 - 178  [0.276] 1250
...      White vs Black: 1331 - 788 - 381  [0.609] 2500
Elo difference: -81.7 +/- 12.8, LOS: 0.0 %, DrawRatio: 15.2 %
```

### 1.1

64 bit: 4083 bytes (+3)

MD5: 4b679bf945d6c68a6c9dc1d5cfffde2f

Transposition table

```py
info depth 1 score cp 28 time 0 nodes 20 pv b1c3
info depth 2 score cp 16 time 0 nodes 94 pv b1c3
info depth 3 score cp 26 time 1 nodes 564 nps 564000 pv b1c3
info depth 4 score cp 16 time 1 nodes 1499 nps 1499000 pv b1c3
info depth 5 score cp -1 time 2 nodes 4630 nps 2315000 pv b1c3
info depth 6 score cp 0 time 4 nodes 12309 nps 3077250 pv b1c3
info depth 7 score cp 7 time 10 nodes 32680 nps 3268000 pv b1c3
info depth 8 score cp 16 time 22 nodes 80877 nps 3676227 pv b1c3
info depth 9 score cp 9 time 57 nodes 228299 nps 4005245 pv b1c3
info depth 10 score cp 17 time 131 nodes 547731 nps 4181152 pv b1c3
info depth 11 score cp 10 time 317 nodes 1359414 nps 4288372 pv b1c3
info depth 12 score cp 9 time 722 nodes 2996764 nps 4150642 pv b1c3
info depth 13 score cp 9 time 1173 nodes 4834673 nps 4121630 pv b1c3
info depth 14 score cp 9 time 2943 nodes 12055907 nps 4096468 pv b1c3
info depth 15 score cp 12 time 5023 nodes 20267558 nps 4034950 pv b1c3
bestmove b1c3
20267558 nodes 4034950 nps
```

```py
Elo   | 44.37 +- 15.54 (95%)
SPRT  | 10.0+0.10s Threads=1 Hash=1MB
LLR   | 2.94 (-2.94, 2.94) [0.00, 5.00]
Games | N: 1480 W: 663 L: 475 D: 342
Penta | [70, 118, 251, 156, 145]
https://gedas.pythonanywhere.com/test/435/
```

```py
Elo   | 38.42 +- 14.14 (95%)
SPRT  | 60.0+0.60s Threads=1 Hash=1MB
LLR   | 2.97 (-2.94, 2.94) [0.00, 5.00]
Games | N: 1698 W: 705 L: 518 D: 475
Penta | [82, 138, 282, 205, 142]
https://gedas.pythonanywhere.com/test/438/
```

### 1.2

64 bit: 4092 bytes (+9)

MD5: e9a7500085476f7af941a25cbb4acf37

Final round of AES for bit mixing

```py
info depth 1 score cp 28 time 1 nodes 20 nps 20000 pv b1c3
info depth 2 score cp 16 time 1 nodes 94 nps 94000 pv b1c3
info depth 3 score cp 26 time 1 nodes 564 nps 564000 pv b1c3
info depth 4 score cp 16 time 1 nodes 1499 nps 1499000 pv b1c3
info depth 5 score cp -1 time 2 nodes 4630 nps 2315000 pv b1c3
info depth 6 score cp 0 time 4 nodes 12309 nps 3077250 pv b1c3
info depth 7 score cp 7 time 9 nodes 32680 nps 3631111 pv b1c3
info depth 8 score cp 16 time 20 nodes 81970 nps 4098500 pv b1c3
info depth 9 score cp 9 time 48 nodes 218779 nps 4557895 pv b1c3
info depth 10 score cp 17 time 134 nodes 614182 nps 4583447 pv b1c3
info depth 11 score cp 13 time 275 nodes 1311030 nps 4767381 pv b1c3
info depth 12 score cp 14 time 626 nodes 2968025 nps 4741253 pv b1c3
info depth 13 score cp 10 time 1393 nodes 6718813 nps 4823268 pv b1c3
info depth 14 score cp 9 time 2325 nodes 11128077 nps 4786269 pv b1c3
info depth 15 score cp 8 time 4300 nodes 20160440 nps 4688474 pv b1c3
bestmove b1c3
20160440 nodes 4688474 nps
```

```py
Elo   | 47.55 +- 15.16 (95%)
SPRT  | 10.0+0.10s Threads=1 Hash=1MB
LLR   | 3.02 (-2.94, 2.94) [0.00, 5.00]
Games | N: 1250 W: 492 L: 322 D: 436
Penta | [28, 121, 229, 147, 100]
https://gedas.pythonanywhere.com/test/459/
```

### 1.3

64 bit: 4088 bytes (-4)

MD5: 91c1fd9a613cd2d4fe610f51fe206afe

Use self as key for final AES round

```py
info depth 1 score cp 28 time 0 nodes 20 pv b1c3
info depth 2 score cp 16 time 0 nodes 94 pv b1c3
info depth 3 score cp 26 time 0 nodes 564 pv b1c3
info depth 4 score cp 16 time 1 nodes 1499 nps 1499000 pv b1c3
info depth 5 score cp -1 time 2 nodes 4630 nps 2315000 pv b1c3
info depth 6 score cp 0 time 4 nodes 12309 nps 3077250 pv b1c3
info depth 7 score cp 7 time 9 nodes 32680 nps 3631111 pv b1c3
info depth 8 score cp 16 time 20 nodes 81991 nps 4099550 pv b1c3
info depth 9 score cp 9 time 50 nodes 228379 nps 4567580 pv b1c3
info depth 10 score cp 17 time 121 nodes 551714 nps 4559619 pv b1c3
info depth 11 score cp 13 time 281 nodes 1318702 nps 4692889 pv b1c3
info depth 12 score cp 11 time 573 nodes 2696571 nps 4706057 pv b1c3
info depth 13 score cp 5 time 2020 nodes 9624691 nps 4764698 pv b1c3
info depth 14 score cp 9 time 3243 nodes 15345945 nps 4732021 pv b1c3
info depth 15 score cp 6 time 6204 nodes 28648795 nps 4617794 pv b1c3
bestmove b1c3
28648795 nodes 4617794 nps
```

```py
Elo   | 0.20 +- 5.66 (95%)
SPRT  | 10.0+0.10s Threads=1 Hash=1MB
LLR   | 0.99 (-2.94, 2.94) [-5.00, 0.00]
Games | N: 8690 W: 2833 L: 2828 D: 3029
Penta | [410, 971, 1571, 990, 403]
https://gedas.pythonanywhere.com/test/462/
```

### 1.4

64 bit: 4087 bytes (-1)

MD5: 877b14290c98cf50b119578c8d614649

Smaller TT entry

```py
info depth 1 score cp 28 time 0 nodes 20 pv b1c3
info depth 2 score cp 16 time 0 nodes 94 pv b1c3
info depth 3 score cp 26 time 0 nodes 564 pv b1c3
info depth 4 score cp 16 time 0 nodes 1499 pv b1c3
info depth 5 score cp -1 time 1 nodes 4630 nps 4630000 pv b1c3
info depth 6 score cp 0 time 3 nodes 12309 nps 4103000 pv b1c3
info depth 7 score cp 7 time 8 nodes 32680 nps 4085000 pv b1c3
info depth 8 score cp 16 time 20 nodes 81868 nps 4093400 pv b1c3
info depth 9 score cp 9 time 52 nodes 226783 nps 4361211 pv b1c3
info depth 10 score cp 17 time 130 nodes 586776 nps 4513661 pv b1c3
info depth 11 score cp 8 time 311 nodes 1439442 nps 4628430 pv b1c3
info depth 12 score cp 12 time 734 nodes 3360741 nps 4578666 pv e2e4
info depth 13 score cp 12 time 1579 nodes 7216729 nps 4570442 pv e2e4
info depth 14 score cp 9 time 2931 nodes 13240076 nps 4517255 pv g1f3
info depth 15 score cp 9 time 4783 nodes 21398552 nps 4473876 pv g1f3
bestmove g1f3
21398552 nodes 4473876 nps
```

```py
Elo   | 5.43 +- 6.06 (95%)
SPRT  | 10.0+0.10s Threads=1 Hash=1MB
LLR   | 3.00 (-2.94, 2.94) [-5.00, 0.00]
Games | N: 7608 W: 2554 L: 2435 D: 2619
Penta | [351, 798, 1414, 863, 378]
https://gedas.pythonanywhere.com/test/465/
```

### 1.5

64 bit: 4077 bytes (-10)

MD5: 9f535c2c9333625f4a8b488f49c572f3

Various size reductions

```py
info depth 1 score cp 28 time 0 nodes 20 pv b1c3
info depth 2 score cp 16 time 0 nodes 94 pv b1c3
info depth 3 score cp 26 time 0 nodes 564 pv b1c3
info depth 4 score cp 16 time 1 nodes 1499 nps 1499000 pv b1c3
info depth 5 score cp -1 time 2 nodes 4630 nps 2315000 pv b1c3
info depth 6 score cp 0 time 4 nodes 12309 nps 3077250 pv b1c3
info depth 7 score cp 7 time 9 nodes 32680 nps 3631111 pv b1c3
info depth 8 score cp 16 time 22 nodes 81868 nps 3721272 pv b1c3
info depth 9 score cp 9 time 54 nodes 226783 nps 4199685 pv b1c3
info depth 10 score cp 17 time 135 nodes 586776 nps 4346488 pv b1c3
info depth 11 score cp 8 time 327 nodes 1439442 nps 4401963 pv b1c3
info depth 12 score cp 12 time 756 nodes 3360741 nps 4445424 pv e2e4
info depth 13 score cp 12 time 1624 nodes 7216729 nps 4443798 pv e2e4
info depth 14 score cp 9 time 2995 nodes 13240076 nps 4420726 pv g1f3
info depth 15 score cp 9 time 4875 nodes 21398552 nps 4389446 pv g1f3
bestmove g1f3
21398552 nodes 4389446 nps
```

### 1.6

64 bit: 4061 bytes (-16)

MD5: 76ec90083751b32fa5837b98b8a7115e

Reduce size by making make_move non-static

```py
info depth 1 score cp 28 time 0 nodes 20 pv b1c3
info depth 2 score cp 16 time 0 nodes 94 pv b1c3
info depth 3 score cp 26 time 1 nodes 564 nps 564000 pv b1c3
info depth 4 score cp 16 time 1 nodes 1499 nps 1499000 pv b1c3
info depth 5 score cp -1 time 2 nodes 4630 nps 2315000 pv b1c3
info depth 6 score cp 0 time 4 nodes 12309 nps 3077250 pv b1c3
info depth 7 score cp 7 time 9 nodes 32680 nps 3631111 pv b1c3
info depth 8 score cp 16 time 20 nodes 81868 nps 4093400 pv b1c3
info depth 9 score cp 9 time 52 nodes 226783 nps 4361211 pv b1c3
info depth 10 score cp 17 time 131 nodes 586776 nps 4479206 pv b1c3
info depth 11 score cp 8 time 319 nodes 1439442 nps 4512357 pv b1c3
info depth 12 score cp 12 time 736 nodes 3360741 nps 4566224 pv e2e4
info depth 13 score cp 12 time 1588 nodes 7216729 nps 4544539 pv e2e4
info depth 14 score cp 9 time 2959 nodes 13240076 nps 4474510 pv g1f3
info depth 15 score cp 9 time 4845 nodes 21398552 nps 4416625 pv g1f3
bestmove g1f3
21398552 nodes 4416625 nps
```

### 1.7

64 bit: 4081 bytes (+20)

MD5: ce943d40ac5921c852f945ed20f7d0b7

Preserve TT move

```py
info depth 1 score cp 28 time 0 nodes 20 pv b1c3
info depth 2 score cp 16 time 0 nodes 94 pv b1c3
info depth 3 score cp 26 time 0 nodes 564 pv b1c3
info depth 4 score cp 16 time 1 nodes 1499 nps 1499000 pv b1c3
info depth 5 score cp -1 time 1 nodes 4628 nps 4628000 pv b1c3
info depth 6 score cp 0 time 4 nodes 12307 nps 3076750 pv b1c3
info depth 7 score cp 7 time 9 nodes 32723 nps 3635888 pv b1c3
info depth 8 score cp 16 time 23 nodes 83877 nps 3646826 pv b1c3
info depth 9 score cp 9 time 61 nodes 231353 nps 3792672 pv b1c3
info depth 10 score cp 13 time 146 nodes 566725 nps 3881678 pv b1c3
info depth 11 score cp 8 time 350 nodes 1378037 nps 3937248 pv b1c3
info depth 12 score cp 12 time 715 nodes 2818878 nps 3942486 pv b1c3
info depth 13 score cp 15 time 1315 nodes 5208000 nps 3960456 pv b1c3
info depth 14 score cp 12 time 2533 nodes 10126337 nps 3997764 pv b1c3
info depth 15 score cp 10 time 6897 nodes 27245341 nps 3950317 pv b1c3
bestmove b1c3
27245341 nodes 3950317 nps
```

```py
Elo   | 31.76 +- 12.14 (95%)
SPRT  | 10.0+0.10s Threads=1 Hash=1MB
LLR   | 2.97 (-2.94, 2.94) [0.00, 5.00]
Games | N: 1854 W: 683 L: 514 D: 657
Penta | [62, 167, 348, 240, 110]
https://gedas.pythonanywhere.com/test/468/
```

### 1.8

64 bit: 4070 bytes (-11)

MD5: 8e1c8c1c8f2f0834dc10903943e53fc9

Smaller AES hashing

```py
info depth 1 score cp 28 time 0 nodes 20 pv b1c3
info depth 2 score cp 16 time 0 nodes 94 pv b1c3
info depth 3 score cp 26 time 0 nodes 564 pv b1c3
info depth 4 score cp 16 time 1 nodes 1511 nps 1511000 pv b1c3
info depth 5 score cp -1 time 2 nodes 4667 nps 2333500 pv b1c3
info depth 6 score cp 0 time 4 nodes 12748 nps 3187000 pv b1c3
info depth 7 score cp 7 time 10 nodes 33800 nps 3380000 pv b1c3
info depth 8 score cp 16 time 22 nodes 82756 nps 3761636 pv b1c3
info depth 9 score cp 9 time 59 nodes 235271 nps 3987644 pv b1c3
info depth 10 score cp 13 time 149 nodes 608078 nps 4081060 pv b1c3
info depth 11 score cp 15 time 342 nodes 1405160 nps 4108654 pv b1c3
info depth 12 score cp 12 time 770 nodes 3323523 nps 4316263 pv b1c3
info depth 13 score cp 8 time 1579 nodes 6882166 nps 4358559 pv b1c3
info depth 14 score cp 11 time 3389 nodes 14652291 nps 4323485 pv b1c3
info depth 15 score cp 7 time 5388 nodes 23209903 nps 4307702 pv b1c3
bestmove b1c3
23209903 nodes 4307702 nps
```

```py
Elo   | -0.68 +- 2.48 (95%)
SPRT  | 10.0+0.10s Threads=1 Hash=1MB
LLR   | 2.96 (-2.94, 2.94) [-5.00, 0.00]
Games | N: 44170 W: 14128 L: 14214 D: 15828
Penta | [1985, 5082, 8035, 5000, 1983]
https://gedas.pythonanywhere.com/test/481/
```

### 1.9

64 bit: 4096 bytes (+26)

MD5: e6075a755bf5f196bade69cca7c458b9

Razoring

```py
info depth 1 score cp 28 time 0 nodes 20 pv b1c3
info depth 2 score cp 16 time 0 nodes 94 pv b1c3
info depth 3 score cp 26 time 0 nodes 564 pv b1c3
info depth 4 score cp 16 time 1 nodes 1511 nps 1511000 pv b1c3
info depth 5 score cp -1 time 2 nodes 4667 nps 2333500 pv b1c3
info depth 6 score cp 0 time 5 nodes 12748 nps 2549600 pv b1c3
info depth 7 score cp 7 time 11 nodes 32441 nps 2949181 pv b1c3
info depth 8 score cp 16 time 23 nodes 80428 nps 3496869 pv b1c3
info depth 9 score cp 9 time 58 nodes 218658 nps 3769965 pv b1c3
info depth 10 score cp 13 time 143 nodes 554702 nps 3879034 pv b1c3
info depth 11 score cp 11 time 367 nodes 1484057 nps 4043752 pv b1c3
info depth 12 score cp 11 time 816 nodes 3422562 nps 4194316 pv b1c3
info depth 13 score cp 12 time 1410 nodes 5784245 nps 4102301 pv b1c3
info depth 14 score cp 12 time 2164 nodes 8676553 nps 4009497 pv b1c3
info depth 15 score cp 11 time 3755 nodes 14802746 nps 3942142 pv b1c3
bestmove b1c3
14802746 nodes 3942142 nps
```

```py
Elo   | 39.67 +- 13.65 (95%)
SPRT  | 10.0+0.10s Threads=1 Hash=1MB
LLR   | 3.10 (-2.94, 2.94) [0.00, 5.00]
Games | N: 1592 W: 625 L: 444 D: 523
Penta | [52, 144, 291, 189, 120]
https://gedas.pythonanywhere.com/test/501/
```

10+0.1:
```py
Score of 4k.c-1.9 vs stash16: 309 - 119 - 86  [0.685] 514
...      4k.c-1.9 playing White: 178 - 48 - 33  [0.751] 259
...      4k.c-1.9 playing Black: 131 - 71 - 53  [0.618] 255
...      White vs Black: 249 - 179 - 86  [0.568] 514
Elo difference: 134.8 +/- 29.1, LOS: 100.0 %, DrawRatio: 16.7 %
```

10+0.1:
```
Score of 4k.c-1.9 vs stash17: 281 - 162 - 57  [0.619] 500
...      4k.c-1.9 playing White: 169 - 58 - 24  [0.721] 251
...      4k.c-1.9 playing Black: 112 - 104 - 33  [0.516] 249
...      White vs Black: 273 - 170 - 57  [0.603] 500
Elo difference: 84.3 +/- 29.4, LOS: 100.0 %, DrawRatio: 11.4 %
```

10+0.1
```py
Score of 4k.c-1.9 vs 4ku-1.1: 217 - 593 - 190  [0.312] 1000
...      4k.c-1.9 playing White: 140 - 257 - 104  [0.383] 501
...      4k.c-1.9 playing Black: 77 - 336 - 86  [0.240] 499
...      White vs Black: 476 - 334 - 190  [0.571] 1000
Elo difference: -137.4 +/- 20.5, LOS: 0.0 %, DrawRatio: 19.0 %
```

### 1.10

64 bit: 4096 bytes (=)

MD5: 0043720135875d9197f1fd242c260c49

Razoring margin 128

```py
info depth 1 score cp 28 time 0 nodes 20 pv b1c3
info depth 2 score cp 16 time 1 nodes 94 nps 94000 pv b1c3
info depth 3 score cp 26 time 1 nodes 564 nps 564000 pv b1c3
info depth 4 score cp 16 time 1 nodes 1511 nps 1511000 pv b1c3
info depth 5 score cp -1 time 2 nodes 4647 nps 2323500 pv b1c3
info depth 6 score cp 0 time 4 nodes 12476 nps 3119000 pv b1c3
info depth 7 score cp 7 time 9 nodes 30358 nps 3373111 pv b1c3
info depth 8 score cp 16 time 21 nodes 72932 nps 3472952 pv b1c3
info depth 9 score cp 9 time 51 nodes 198575 nps 3893627 pv b1c3
info depth 10 score cp 13 time 126 nodes 498567 nps 3956880 pv b1c3
info depth 11 score cp 11 time 323 nodes 1317735 nps 4079674 pv b1c3
info depth 12 score cp 12 time 614 nodes 2582193 nps 4205526 pv b1c3
info depth 13 score cp 8 time 1180 nodes 5082739 nps 4307405 pv b1c3
info depth 14 score cp 7 time 2873 nodes 12403950 nps 4317420 pv b1c3
info depth 15 score cp 10 time 5933 nodes 25158467 nps 4240429 pv g1f3
bestmove g1f3
25158467 nodes 4240429 nps
```

```py
Elo   | 15.88 +- 8.30 (95%)
SPRT  | 10.0+0.10s Threads=1 Hash=1MB
LLR   | 2.98 (-2.94, 2.94) [0.00, 5.00]
Games | N: 3920 W: 1370 L: 1191 D: 1359
Penta | [140, 413, 754, 434, 219]
https://gedas.pythonanywhere.com/test/512/
```

```py
Elo   | 20.79 +- 9.60 (95%)
SPRT  | 60.0+0.60s Threads=1 Hash=1MB
LLR   | 2.95 (-2.94, 2.94) [0.00, 5.00]
Games | N: 2828 W: 977 L: 808 D: 1043
Penta | [89, 301, 519, 362, 143]
https://gedas.pythonanywhere.com/test/514/
```

### 1.11

64 bit: 4077 bytes (-19)

MD5: 993bd168a80ee9a75c945b4f9c330721

Smaller bestmove preservation

```py
info depth 1 score cp 28 time 0 nodes 20 pv b1c3
info depth 2 score cp 16 time 1 nodes 94 nps 94000 pv b1c3
info depth 3 score cp 26 time 1 nodes 564 nps 564000 pv b1c3
info depth 4 score cp 16 time 1 nodes 1511 nps 1511000 pv b1c3
info depth 5 score cp -1 time 2 nodes 4647 nps 2323500 pv b1c3
info depth 6 score cp 0 time 5 nodes 12476 nps 2495200 pv b1c3
info depth 7 score cp 7 time 11 nodes 30471 nps 2770090 pv b1c3
info depth 8 score cp 16 time 23 nodes 73368 nps 3189913 pv b1c3
info depth 9 score cp 9 time 59 nodes 200904 nps 3405152 pv b1c3
info depth 10 score cp 13 time 139 nodes 484938 nps 3488762 pv b1c3
info depth 11 score cp 8 time 346 nodes 1230336 nps 3555884 pv b1c3
info depth 12 score cp 12 time 662 nodes 2419808 nps 3655299 pv b1c3
info depth 13 score cp 8 time 1262 nodes 4731002 nps 3748812 pv b1c3
info depth 14 score cp 9 time 2056 nodes 7801771 nps 3794635 pv b1c3
info depth 15 score cp 9 time 3215 nodes 11997595 nps 3731755 pv b1c3
bestmove b1c3
11997595 nodes 3731755 nps
```

```py
Elo   | -7.85 +- 5.45 (95%)
SPRT  | 10.0+0.10s Threads=1 Hash=1MB
LLR   | -2.95 (-2.94, 2.94) [-5.00, 0.00]
Games | N: 9248 W: 2880 L: 3089 D: 3279
Penta | [460, 1076, 1685, 1019, 384]
https://gedas.pythonanywhere.com/test/517/
```

### 1.12

64 bit: 4050 bytes (-27)

MD5: 42593f0e6a4cffa1418b8e01989a987c

Fix move comparison

```py
info depth 1 score cp 28 time 0 nodes 20 pv b1c3
info depth 2 score cp 16 time 0 nodes 94 pv b1c3
info depth 3 score cp 26 time 1 nodes 564 nps 564000 pv b1c3
info depth 4 score cp 16 time 1 nodes 1511 nps 1511000 pv b1c3
info depth 5 score cp -1 time 2 nodes 4647 nps 2323500 pv b1c3
info depth 6 score cp 0 time 5 nodes 12476 nps 2495200 pv b1c3
info depth 7 score cp 7 time 10 nodes 30471 nps 3047100 pv b1c3
info depth 8 score cp 16 time 23 nodes 73052 nps 3176173 pv b1c3
info depth 9 score cp 9 time 55 nodes 193693 nps 3521690 pv b1c3
info depth 10 score cp 16 time 156 nodes 572116 nps 3667410 pv b1c3
info depth 11 score cp 8 time 407 nodes 1468056 nps 3607017 pv b1c3
info depth 12 score cp 12 time 817 nodes 3119910 nps 3818739 pv b1c3
info depth 13 score cp 10 time 1471 nodes 5760790 nps 3916240 pv b1c3
info depth 14 score cp 16 time 3097 nodes 12302524 nps 3972400 pv b1c3
info depth 15 score cp 10 time 5431 nodes 21422681 nps 3944518 pv b1c3
bestmove b1c3
21422681 nodes 3944518 nps
```

```py
Elo   | 13.00 +- 8.62 (95%)
SPRT  | 10.0+0.10s Threads=1 Hash=1MB
LLR   | 2.97 (-2.94, 2.94) [-5.00, 0.00]
Games | N: 3716 W: 1291 L: 1152 D: 1273
Penta | [157, 384, 661, 475, 181]
https://gedas.pythonanywhere.com/test/529/
```

### 1.13

64 bit: 4093 bytes (+43)

MD5: 63aed21c47366d9ae758f3c389ef8b99

* Add open file evaluation
* Remove killer heuristic

```py
info depth 1 score cp 28 time 0 nodes 20 pv b1c3
info depth 2 score cp 16 time 0 nodes 94 pv b1c3
info depth 3 score cp 24 time 0 nodes 574 pv b1c3
info depth 4 score cp 16 time 0 nodes 1523 pv b1c3
info depth 5 score cp 0 time 1 nodes 4674 nps 4674000 pv b1c3
info depth 6 score cp 2 time 3 nodes 12687 nps 4229000 pv b1c3
info depth 7 score cp 6 time 10 nodes 34616 nps 3461600 pv b1c3
info depth 8 score cp 15 time 24 nodes 87408 nps 3642000 pv b1c3
info depth 9 score cp 13 time 62 nodes 245013 nps 3951822 pv b1c3
info depth 10 score cp 12 time 185 nodes 743094 nps 4016724 pv b1c3
info depth 11 score cp 12 time 364 nodes 1461810 nps 4015961 pv b1c3
info depth 12 score cp 14 time 735 nodes 3040375 nps 4136564 pv b1c3
info depth 13 score cp 3 time 1601 nodes 6655101 nps 4156840 pv b1c3
info depth 14 score cp 4 time 2714 nodes 11241642 nps 4142093 pv b1c3
info depth 15 score cp 4 time 4434 nodes 18369730 nps 4142925 pv b1c3
bestmove b1c3
18369730 nodes 4142925 nps
```

```py
Elo   | 22.46 +- 10.21 (95%)
SPRT  | 10.0+0.10s Threads=1 Hash=1MB
LLR   | 3.05 (-2.94, 2.94) [0.00, 5.00]
Games | N: 2912 W: 1089 L: 901 D: 922
Penta | [122, 298, 499, 344, 193]
https://gedas.pythonanywhere.com/test/552/
```

```py
Elo   | -1.25 +- 4.10 (95%)
SPRT  | 60.0+0.60s Threads=1 Hash=1MB
LLR   | -2.95 (-2.94, 2.94) [0.00, 5.00]
Games | N: 17460 W: 5841 L: 5904 D: 5715
Penta | [912, 1908, 3158, 1835, 917]
https://gedas.pythonanywhere.com/test/554/
```

### 1.14

64 bit: 4096 bytes (+3)

MD5: b7286fe79a667bdc996837d25a6dce7f

Doubled pawns through open files

```py
info depth 1 score cp 23 time 0 nodes 20 pv b1c3
info depth 2 score cp 16 time 0 nodes 94 pv b1c3
info depth 3 score cp 21 time 0 nodes 574 pv b1c3
info depth 4 score cp 16 time 1 nodes 1529 nps 1529000 pv b1c3
info depth 5 score cp 0 time 2 nodes 5493 nps 2746500 pv b1c3
info depth 6 score cp 9 time 6 nodes 18384 nps 3064000 pv f2f4
info depth 7 score cp 10 time 17 nodes 57281 nps 3369470 pv g1f3
info depth 8 score cp 16 time 29 nodes 108606 nps 3745034 pv g1f3
info depth 9 score cp 10 time 63 nodes 247680 nps 3931428 pv g1f3
info depth 10 score cp 10 time 154 nodes 612289 nps 3975902 pv g1f3
info depth 11 score cp 10 time 302 nodes 1204177 nps 3987341 pv g1f3
info depth 12 score cp 6 time 906 nodes 3739840 nps 4127858 pv g1f3
info depth 13 score cp 8 time 1626 nodes 6751203 nps 4152031 pv g1f3
info depth 14 score cp 8 time 3252 nodes 13451364 nps 4136335 pv g1f3
bestmove g1f3
13451364 nodes 4136335 nps
```

```py
Elo   | 23.04 +- 10.42 (95%)
SPRT  | 10.0+0.10s Threads=1 Hash=1MB
LLR   | 2.98 (-2.94, 2.94) [0.00, 5.00]
Games | N: 2764 W: 1039 L: 856 D: 869
Penta | [124, 259, 480, 348, 171]
https://gedas.pythonanywhere.com/test/555/
```

10+0.1
```py
Score of 4k.c-1.14 vs stash16: 642 - 223 - 135  [0.710] 1000
...      4k.c-1.14 playing White: 374 - 75 - 52  [0.798] 501
...      4k.c-1.14 playing Black: 268 - 148 - 83  [0.620] 499
...      White vs Black: 522 - 343 - 135  [0.590] 1000
Elo difference: 155.1 +/- 21.7, LOS: 100.0 %, DrawRatio: 13.5 %
```

10+0.1
```py
Score of 4k.c-1.14 vs stash17: 617 - 252 - 131  [0.682] 1000
...      4k.c-1.14 playing White: 362 - 91 - 48  [0.770] 501
...      4k.c-1.14 playing Black: 255 - 161 - 83  [0.594] 499
...      White vs Black: 523 - 346 - 131  [0.589] 1000
Elo difference: 132.9 +/- 21.3, LOS: 100.0 %, DrawRatio: 13.1 %
```

10+0.1
```py
Score of 4k.c-1.14 vs stash18: 503 - 329 - 168  [0.587] 1000
...      4k.c-1.14 playing White: 297 - 117 - 86  [0.680] 500
...      4k.c-1.14 playing Black: 206 - 212 - 82  [0.494] 500
...      White vs Black: 509 - 323 - 168  [0.593] 1000
Elo difference: 61.1 +/- 19.9, LOS: 100.0 %, DrawRatio: 16.8 %
```

10+0.1
```py
Score of 4k.c-1.14 vs stash19: 383 - 454 - 163  [0.465] 1000
...      4k.c-1.14 playing White: 244 - 178 - 80  [0.566] 502
...      4k.c-1.14 playing Black: 139 - 276 - 83  [0.362] 498
...      White vs Black: 520 - 317 - 163  [0.602] 1000
Elo difference: -24.7 +/- 19.7, LOS: 0.7 %, DrawRatio: 16.3 %
```

10+0.1
```py
Score of 4k.c-1.14 vs 4ku-1.1: 501 - 1116 - 383  [0.346] 2000
...      4k.c-1.14 playing White: 307 - 456 - 237  [0.425] 1000
...      4k.c-1.14 playing Black: 194 - 660 - 146  [0.267] 1000
...      White vs Black: 967 - 650 - 383  [0.579] 2000
Elo difference: -110.4 +/- 14.2, LOS: 0.0 %, DrawRatio: 19.1 %
```

### 1.15

64 bit: 4082 bytes (-14)

MD5: ad7c39ff26ee23568a50288100162225

Simplified LMR

```py
info depth 1 score cp 23 time 0 nodes 20 pv b1c3
info depth 2 score cp 16 time 0 nodes 85 pv b1c3
info depth 3 score cp 21 time 0 nodes 330 pv b1c3
info depth 4 score cp 16 time 1 nodes 945 nps 945000 pv b1c3
info depth 5 score cp 0 time 2 nodes 3017 nps 1508500 pv b1c3
info depth 6 score cp 9 time 6 nodes 10878 nps 1813000 pv f2f4
info depth 7 score cp 0 time 18 nodes 31042 nps 1724555 pv g1f3
info depth 8 score cp 9 time 35 nodes 62285 nps 1779571 pv g1f3
info depth 9 score cp 16 time 88 nodes 183529 nps 2085556 pv d2d4
info depth 10 score cp 10 time 180 nodes 403285 nps 2240472 pv d2d4
info depth 11 score cp 16 time 570 nodes 1322973 nps 2321005 pv g1f3
info depth 12 score cp 9 time 1041 nodes 2484109 nps 2386271 pv g1f3
info depth 13 score cp 9 time 2153 nodes 4623207 nps 2147332 pv g1f3
info depth 14 score cp 8 time 3605 nodes 8238111 nps 2285190 pv g1f3
info depth 15 score cp 9 time 6459 nodes 15453936 nps 2392620 pv g1f3
bestmove g1f3
15453936 nodes 2392620 nps
```

```py
Elo   | 30.61 +- 12.64 (95%)
SPRT  | 10.0+0.10s Threads=1 Hash=1MB
LLR   | 3.02 (-2.94, 2.94) [-5.00, 0.00]
Games | N: 1764 W: 641 L: 486 D: 637
Penta | [68, 158, 304, 255, 97]
https://gedas.pythonanywhere.com/test/559/
```

```py
Elo   | -4.84 +- 5.83 (95%)
SPRT  | 60.0+0.60s Threads=1 Hash=1MB
LLR   | -2.97 (-2.94, 2.94) [0.00, 5.00]
Games | N: 8182 W: 2593 L: 2707 D: 2882
Penta | [408, 921, 1498, 905, 359]
https://gedas.pythonanywhere.com/test/560/
```

### 1.16

64 bit: 4075 bytes (-7)

MD5: 350d45ca7b880717ef0aaf41b6288267

Reduce size

### 1.17

64 bit: 4093 bytes (+18)

MD5: cb938b3547b8bbcef7bed262acee9799

Killer heuristic

```py
info depth 1 score cp 23 time 0 nodes 20 pv b1c3
info depth 2 score cp 16 time 1 nodes 85 nps 85000 pv b1c3
info depth 3 score cp 21 time 1 nodes 341 nps 341000 pv b1c3
info depth 4 score cp 16 time 1 nodes 954 nps 954000 pv b1c3
info depth 5 score cp 0 time 2 nodes 3016 nps 1508000 pv b1c3
info depth 6 score cp 9 time 5 nodes 11020 nps 2204000 pv f2f4
info depth 7 score cp 8 time 10 nodes 31681 nps 3168100 pv b1c3
info depth 8 score cp 9 time 19 nodes 61675 nps 3246052 pv b1c3
info depth 9 score cp 10 time 42 nodes 147141 nps 3503357 pv b1c3
info depth 10 score cp 10 time 100 nodes 357766 nps 3577660 pv b1c3
info depth 11 score cp 8 time 278 nodes 998330 nps 3591115 pv b1c3
info depth 12 score cp 8 time 562 nodes 2059565 nps 3664706 pv b1c3
info depth 13 score cp 9 time 1067 nodes 4011012 nps 3759149 pv b1c3
info depth 14 score cp 11 time 1796 nodes 6929920 nps 3858530 pv b1c3
info depth 15 score cp 5 time 4525 nodes 17634300 nps 3897082 pv b1c3
bestmove b1c3
17634300 nodes 3897082 nps
```

```py
Elo   | 21.27 +- 9.84 (95%)
SPRT  | 10.0+0.10s Threads=1 Hash=1MB
LLR   | 2.99 (-2.94, 2.94) [0.00, 5.00]
Games | N: 2928 W: 1061 L: 882 D: 985
Penta | [124, 269, 538, 370, 163]
https://gedas.pythonanywhere.com/test/564/
```

```py
Elo   | 53.62 +- 15.81 (95%)
SPRT  | 60.0+0.60s Threads=1 Hash=1MB
LLR   | 2.95 (-2.94, 2.94) [0.00, 5.00]
Games | N: 1058 W: 417 L: 255 D: 386
Penta | [22, 91, 188, 159, 69]
https://gedas.pythonanywhere.com/test/565/
```

```py
Score of 4k.c-1.17 vs 4ku-1.1: 1079 - 2151 - 770  [0.366] 4000
...      4k.c-1.17 playing White: 647 - 892 - 462  [0.439] 2001
...      4k.c-1.17 playing Black: 432 - 1259 - 308  [0.293] 1999
...      White vs Black: 1906 - 1324 - 770  [0.573] 4000
Elo difference: -95.4 +/- 9.9, LOS: 0.0 %, DrawRatio: 19.3 %
```

### 1.18

64 bit: 4089 bytes (-4)

MD5: 2bde72488af5282c7f7b1d25fde13a69

Don't consider castling for hash

```py
info depth 1 score cp 23 time 0 nodes 20 pv b1c3
info depth 2 score cp 16 time 0 nodes 85 pv b1c3
info depth 3 score cp 21 time 0 nodes 341 pv b1c3
info depth 4 score cp 16 time 1 nodes 945 nps 945000 pv b1c3
info depth 5 score cp 0 time 1 nodes 2990 nps 2990000 pv b1c3
info depth 6 score cp 9 time 4 nodes 10774 nps 2693500 pv f2f4
info depth 7 score cp 8 time 9 nodes 30823 nps 3424777 pv b1c3
info depth 8 score cp 9 time 17 nodes 59481 nps 3498882 pv b1c3
info depth 9 score cp 10 time 36 nodes 130792 nps 3633111 pv b1c3
info depth 10 score cp 12 time 78 nodes 297222 nps 3810538 pv b1c3
info depth 11 score cp 13 time 196 nodes 762904 nps 3892367 pv b1c3
info depth 12 score cp 9 time 480 nodes 1880981 nps 3918710 pv b1c3
info depth 13 score cp 6 time 1111 nodes 4439740 nps 3996165 pv b1c3
info depth 14 score cp 6 time 1996 nodes 8044561 nps 4030341 pv b1c3
info depth 15 score cp 9 time 3547 nodes 14221640 nps 4009484 pv b1c3
bestmove b1c3
14221640 nodes 4009484 nps
```

```py
Elo   | -0.70 +- 2.41 (95%)
SPRT  | 10.0+0.10s Threads=1 Hash=1MB
LLR   | 2.99 (-2.94, 2.94) [-5.00, 0.00]
Games | N: 45916 W: 14637 L: 14730 D: 16549
Penta | [2013, 5208, 8596, 5141, 2000]
https://gedas.pythonanywhere.com/test/575/
```

### 1.19

64 bit: 4081 bytes (-8)

MD5: bc3bedb9923b8db85dbcf886705a8c81

Use search stack for move parsing

### 1.20

64 bit: 4075 bytes (-6)

MD5: 5fbfa814a85caf679c6de9a766ede0bc

Store move count on search stack

```py
info depth 1 score cp 23 time 0 nodes 20 pv b1c3
info depth 2 score cp 16 time 0 nodes 85 pv b1c3
info depth 3 score cp 21 time 0 nodes 341 pv b1c3
info depth 4 score cp 16 time 0 nodes 945 pv b1c3
info depth 5 score cp 0 time 1 nodes 2990 nps 2990000 pv b1c3
info depth 6 score cp 9 time 3 nodes 10774 nps 3591333 pv f2f4
info depth 7 score cp 8 time 8 nodes 30823 nps 3852875 pv b1c3
info depth 8 score cp 9 time 15 nodes 59481 nps 3965400 pv b1c3
info depth 9 score cp 10 time 33 nodes 130792 nps 3963393 pv b1c3
info depth 10 score cp 12 time 73 nodes 297222 nps 4071534 pv b1c3
info depth 11 score cp 13 time 186 nodes 762904 nps 4101634 pv b1c3
info depth 12 score cp 9 time 444 nodes 1880981 nps 4236443 pv b1c3
info depth 13 score cp 6 time 1044 nodes 4439740 nps 4252624 pv b1c3
info depth 14 score cp 6 time 1943 nodes 8044561 nps 4140278 pv b1c3
info depth 15 score cp 9 time 3490 nodes 14221640 nps 4074968 pv b1c3
bestmove b1c3
14221640 nodes 4074968 nps
```

### 1.21

64 bit: 4095 bytes (+20)

MD5: b5df64aa66b074aeeef95a440d88391e

Bishop pair eval

```py
info depth 1 score cp 23 time 0 nodes 20 pv b1c3
info depth 2 score cp 16 time 0 nodes 85 pv b1c3
info depth 3 score cp 22 time 0 nodes 341 pv b1c3
info depth 4 score cp 16 time 1 nodes 945 nps 945000 pv b1c3
info depth 5 score cp -1 time 1 nodes 2991 nps 2991000 pv b1c3
info depth 6 score cp 11 time 4 nodes 11268 nps 2817000 pv f2f4
info depth 7 score cp 5 time 10 nodes 29429 nps 2942900 pv g1f3
info depth 8 score cp 16 time 17 nodes 53639 nps 3155235 pv g1f3
info depth 9 score cp 8 time 41 nodes 138198 nps 3370682 pv g1f3
info depth 10 score cp 12 time 92 nodes 325207 nps 3534858 pv g1f3
info depth 11 score cp 12 time 217 nodes 798686 nps 3680580 pv g1f3
info depth 12 score cp 12 time 440 nodes 1640973 nps 3729484 pv g1f3
info depth 13 score cp 7 time 1047 nodes 3944334 nps 3767272 pv g1f3
info depth 14 score cp 8 time 1893 nodes 7122333 nps 3762458 pv g1f3
info depth 15 score cp 8 time 2785 nodes 10640446 nps 3820626 pv g1f3
bestmove g1f3
10640446 nodes 3820626 nps
```

```py
Elo   | 10.51 +- 6.56 (95%)
SPRT  | 10.0+0.10s Threads=1 Hash=1MB
LLR   | 2.95 (-2.94, 2.94) [0.00, 5.00]
Games | N: 6382 W: 2130 L: 1937 D: 2315
Penta | [268, 674, 1159, 777, 313]
https://gedas.pythonanywhere.com/test/581/
```

```py
Elo   | 13.31 +- 7.40 (95%)
SPRT  | 60.0+0.60s Threads=1 Hash=1MB
LLR   | 2.96 (-2.94, 2.94) [0.00, 5.00]
Games | N: 4596 W: 1487 L: 1311 D: 1798
Penta | [156, 502, 837, 616, 187]
https://gedas.pythonanywhere.com/test/583/
```

```py
Score of 4k.c-1.21 vs 4ku-1.1: 555 - 1049 - 396  [0.377] 2000
...      4k.c-1.21 playing White: 345 - 448 - 207  [0.449] 1000
...      4k.c-1.21 playing Black: 210 - 601 - 189  [0.304] 1000
...      White vs Black: 946 - 658 - 396  [0.572] 2000
Elo difference: -87.6 +/- 13.9, LOS: 0.0 %, DrawRatio: 19.8 %
```

### 1.22

64 bit: 4093 bytes (-2)

MD5: d0a2a019d4da516344105e13a807df54

4 byte (32 bit) partial hash in TT entry

```py
info depth 1 score cp 23 time 0 nodes 20 pv b1c3
info depth 2 score cp 16 time 0 nodes 85 pv b1c3
info depth 3 score cp 22 time 0 nodes 341 pv b1c3
info depth 4 score cp 16 time 1 nodes 945 nps 945000 pv b1c3
info depth 5 score cp -1 time 1 nodes 2991 nps 2991000 pv b1c3
info depth 6 score cp 11 time 4 nodes 11268 nps 2817000 pv f2f4
info depth 7 score cp 5 time 9 nodes 29429 nps 3269888 pv g1f3
info depth 8 score cp 16 time 16 nodes 53508 nps 3344250 pv g1f3
info depth 9 score cp 8 time 38 nodes 137219 nps 3611026 pv g1f3
info depth 10 score cp 16 time 87 nodes 325056 nps 3736275 pv g1f3
info depth 11 score cp 6 time 233 nodes 883223 nps 3790656 pv g1f3
info depth 12 score cp 9 time 498 nodes 1931142 nps 3877795 pv g1f3
info depth 13 score cp 14 time 1025 nodes 4095438 nps 3995549 pv g1f3
info depth 14 score cp 11 time 2071 nodes 8284797 nps 4000384 pv g1f3
info depth 15 score cp 8 time 3852 nodes 15301998 nps 3972481 pv g1f3
bestmove g1f3
15301998 nodes 3972481 nps
```

```py
Elo   | 4.09 +- 5.34 (95%)
SPRT  | 10.0+0.10s Threads=1 Hash=1MB
LLR   | 3.02 (-2.94, 2.94) [-5.00, 0.00]
Games | N: 9164 W: 2971 L: 2863 D: 3330
Penta | [367, 1025, 1727, 1059, 404]
https://gedas.pythonanywhere.com/test/589/
```

### 1.23

64 bit: 4093 bytes (=)

MD5: 3e2e80bffc328a7551cd05d6e1af25c6

RFP margin 42

```py
info depth 1 score cp 23 time 0 nodes 20 pv b1c3
info depth 2 score cp 16 time 0 nodes 85 pv b1c3
info depth 3 score cp 22 time 0 nodes 341 pv b1c3
info depth 4 score cp 16 time 0 nodes 945 pv b1c3
info depth 5 score cp -1 time 1 nodes 2922 nps 2922000 pv b1c3
info depth 6 score cp 11 time 3 nodes 7519 nps 2506333 pv b1c3
info depth 7 score cp 8 time 6 nodes 19419 nps 3236500 pv b1c3
info depth 8 score cp 16 time 14 nodes 46952 nps 3353714 pv b1c3
info depth 9 score cp 8 time 33 nodes 125480 nps 3802424 pv b1c3
info depth 10 score cp 13 time 78 nodes 306534 nps 3929923 pv b1c3
info depth 11 score cp 12 time 199 nodes 798114 nps 4010623 pv b1c3
info depth 12 score cp 15 time 517 nodes 2048520 nps 3962321 pv b1c3
info depth 13 score cp 15 time 987 nodes 3942862 nps 3994794 pv b1c3
info depth 14 score cp 11 time 2037 nodes 8143612 nps 3997845 pv b1c3
info depth 15 score cp 17 time 3654 nodes 14373129 nps 3933532 pv b1c3
bestmove b1c3
14373129 nodes 3932456 nps
```

```py
Elo   | 4.35 +- 3.45 (95%)
SPRT  | 10.0+0.10s Threads=1 Hash=1MB
LLR   | 3.00 (-2.94, 2.94) [0.00, 5.00]
Games | N: 22120 W: 7153 L: 6876 D: 8091
Penta | [888, 2487, 4126, 2578, 981]
https://gedas.pythonanywhere.com/test/603/
```

```py
Elo   | 4.54 +- 3.59 (95%)
SPRT  | 60.0+0.60s Threads=1 Hash=1MB
LLR   | 2.96 (-2.94, 2.94) [0.00, 5.00]
Games | N: 19448 W: 6150 L: 5896 D: 7402
Penta | [714, 2191, 3749, 2267, 803]
https://gedas.pythonanywhere.com/test/604/
```

### 1.24

64 bit: 4093 bytes (=)

MD5: ee3602f77b3e11db85f744bba11d4a70

2 byte (16 bit) partial hash in TT entry

```py
info depth 1 score cp 23 time 0 nodes 20 pv b1c3
info depth 2 score cp 16 time 0 nodes 85 pv b1c3
info depth 3 score cp 22 time 0 nodes 341 pv b1c3
info depth 4 score cp 16 time 1 nodes 945 nps 945000 pv b1c3
info depth 5 score cp -1 time 1 nodes 2922 nps 2922000 pv b1c3
info depth 6 score cp 11 time 3 nodes 7519 nps 2506333 pv b1c3
info depth 7 score cp 8 time 6 nodes 19419 nps 3236500 pv b1c3
info depth 8 score cp 16 time 14 nodes 47250 nps 3375000 pv b1c3
info depth 9 score cp 8 time 32 nodes 123579 nps 3861843 pv b1c3
info depth 10 score cp 16 time 76 nodes 308505 nps 4059276 pv b1c3
info depth 11 score cp 9 time 187 nodes 781247 nps 4177791 pv b1c3
info depth 12 score cp 15 time 448 nodes 1838888 nps 4104660 pv b1c3
info depth 13 score cp 15 time 758 nodes 3123086 nps 4120166 pv b1c3
info depth 14 score cp 14 time 1659 nodes 6776785 nps 4084861 pv b1c3
info depth 15 score cp 8 time 4176 nodes 16645284 nps 3985939 pv b1c3
bestmove b1c3
16645284 nodes 3985939 nps
```

```py
Elo   | 1.23 +- 1.78 (95%)
SPRT  | 10.0+0.10s Threads=1 Hash=1MB
LLR   | -2.94 (-2.94, 2.94) [0.00, 5.00]
Games | N: 82592 W: 26355 L: 26062 D: 30175
Penta | [3448, 9298, 15625, 9363, 3562]
https://gedas.pythonanywhere.com/test/590/
```

```py
Elo   | 15.37 +- 8.08 (95%)
SPRT  | 60.0+0.60s Threads=1 Hash=1MB
LLR   | 2.97 (-2.94, 2.94) [0.00, 5.00]
Games | N: 3982 W: 1317 L: 1141 D: 1524
Penta | [143, 405, 770, 479, 194]
https://gedas.pythonanywhere.com/test/605/
```

### 1.25

64 bit: 4093 bytes (=)

MD5: ee7aa97553d9524fd336dbc0147405c7

Hash default 64 MB

```py
info depth 1 score cp 23 time 0 nodes 20 pv b1c3
info depth 2 score cp 16 time 1 nodes 85 nps 85000 pv b1c3
info depth 3 score cp 22 time 1 nodes 341 nps 341000 pv b1c3
info depth 4 score cp 16 time 1 nodes 945 nps 945000 pv b1c3
info depth 5 score cp -1 time 2 nodes 2922 nps 1461000 pv b1c3
info depth 6 score cp 11 time 3 nodes 7519 nps 2506333 pv b1c3
info depth 7 score cp 8 time 7 nodes 19419 nps 2774142 pv b1c3
info depth 8 score cp 16 time 17 nodes 47345 nps 2785000 pv b1c3
info depth 9 score cp 8 time 43 nodes 125534 nps 2919395 pv b1c3
info depth 10 score cp 13 time 99 nodes 311094 nps 3142363 pv b1c3
info depth 11 score cp 9 time 239 nodes 822879 nps 3443008 pv b1c3
info depth 12 score cp 15 time 573 nodes 2046255 nps 3571125 pv b1c3
info depth 13 score cp 10 time 1495 nodes 5377441 nps 3596950 pv b1c3
info depth 14 score cp 8 time 3689 nodes 13034952 nps 3533464 pv b1c3
info depth 15 score cp 7 time 8335 nodes 29273342 nps 3512098 pv b1c3
bestmove b1c3
29273342 nodes 3512098 nps
```

```py
Elo   | 3.18 +- 2.48 (95%)
SPRT  | 10.0+0.10s Threads=1 Hash=1MB
LLR   | 3.06 (-2.94, 2.94) [0.00, 5.00]
Games | N: 42662 W: 13791 L: 13400 D: 15471
Penta | [1755, 4753, 8048, 4896, 1879]
https://gedas.pythonanywhere.com/test/606/
```

```py
Elo   | 33.84 +- 12.41 (95%)
SPRT  | 60.0+0.60s Threads=1 Hash=1MB
LLR   | 2.95 (-2.94, 2.94) [0.00, 5.00]
Games | N: 1658 W: 581 L: 420 D: 657
Penta | [41, 169, 293, 240, 86]
https://gedas.pythonanywhere.com/test/607/
```

### 1.26

64 bit: 4086 bytes (-7)

MD5: 20b9abd15525001a13211ae9a79b8df4

Smaller partial TT key computing

```py
info depth 1 score cp 23 time 0 nodes 20 pv b1c3
info depth 2 score cp 16 time 0 nodes 85 pv b1c3
info depth 3 score cp 22 time 0 nodes 341 pv b1c3
info depth 4 score cp 16 time 1 nodes 945 nps 945000 pv b1c3
info depth 5 score cp -1 time 1 nodes 2922 nps 2922000 pv b1c3
info depth 6 score cp 11 time 3 nodes 7519 nps 2506333 pv b1c3
info depth 7 score cp 8 time 7 nodes 19419 nps 2774142 pv b1c3
info depth 8 score cp 16 time 17 nodes 47345 nps 2785000 pv b1c3
info depth 9 score cp 8 time 43 nodes 125534 nps 2919395 pv b1c3
info depth 10 score cp 13 time 99 nodes 311094 nps 3142363 pv b1c3
info depth 11 score cp 9 time 237 nodes 822879 nps 3472063 pv b1c3
info depth 12 score cp 15 time 573 nodes 2046255 nps 3571125 pv b1c3
info depth 13 score cp 10 time 1471 nodes 5377441 nps 3655636 pv b1c3
info depth 14 score cp 8 time 3498 nodes 13034952 nps 3726401 pv b1c3
info depth 15 score cp 7 time 7824 nodes 29273342 nps 3741480 pv b1c3
bestmove b1c3
29273342 nodes 3741480 nps
```

### 2.0

Equivalent to version 1.26

Final version without compression

### 2.1

64 bit: 3970 bytes (-116)

MD5: cab3ac3d7d9135c51f247e1033699062

LZ4 compression

Thanks to:
* **zamfofex** for helping me throughout the implementation of this
* **ONE_RANDOM_HUMAN** for the idea of loading the executable into the same virtual address space

### 2.2

64 bit: 4089 bytes (+119)

MD5: f223db1e5a7107148acc0bcb1a3663fe

Null move pruning

```py
info depth 1 score cp 23 time 0 nodes 20 pv b1c3
info depth 2 score cp 16 time 0 nodes 85 pv b1c3
info depth 3 score cp 22 time 1 nodes 341 nps 341000 pv b1c3
info depth 4 score cp 16 time 1 nodes 945 nps 945000 pv b1c3
info depth 5 score cp -1 time 2 nodes 2922 nps 1461000 pv b1c3
info depth 6 score cp 11 time 3 nodes 6898 nps 2299333 pv b1c3
info depth 7 score cp 7 time 7 nodes 20248 nps 2892571 pv b1c3
info depth 8 score cp 13 time 14 nodes 37518 nps 2679857 pv b1c3
info depth 9 score cp 8 time 38 nodes 99642 nps 2622157 pv b1c3
info depth 10 score cp 14 time 61 nodes 166560 nps 2730491 pv b1c3
info depth 11 score cp 9 time 131 nodes 398901 nps 3045045 pv b1c3
info depth 12 score cp 11 time 271 nodes 898344 nps 3314922 pv b1c3
info depth 13 score cp 9 time 601 nodes 2088817 nps 3475569 pv b1c3
info depth 14 score cp 10 time 1207 nodes 4259169 nps 3528723 pv b1c3
info depth 15 score cp 14 time 2783 nodes 9986438 nps 3588371 pv b1c3
info depth 16 score cp 15 time 4612 nodes 16464738 nps 3569977 pv b1c3
bestmove b1c3
16464738 nodes 3569977 nps
```

```py
Elo   | 70.69 +- 17.99 (95%)
SPRT  | 10.0+0.10s Threads=1 Hash=1MB
LLR   | 3.02 (-2.94, 2.94) [0.00, 5.00]
Games | N: 842 W: 361 L: 192 D: 289
Penta | [15, 58, 159, 121, 68]
https://gedas.pythonanywhere.com/test/615/
```

```py
Elo   | 77.57 +- 19.29 (95%)
SPRT  | 60.0+0.60s Threads=1 Hash=1MB
LLR   | 2.96 (-2.94, 2.94) [0.00, 5.00]
Games | N: 774 W: 329 L: 159 D: 286
Penta | [14, 54, 134, 118, 67]
https://gedas.pythonanywhere.com/test/623/
```

```py
Score of 4k.c-2.2 vs 4ku-1.1: 592 - 986 - 422  [0.402] 2000
...      4k.c-2.2 playing White: 379 - 387 - 234  [0.496] 1000
...      4k.c-2.2 playing Black: 213 - 599 - 188  [0.307] 1000
...      White vs Black: 978 - 600 - 422  [0.595] 2000
Elo difference: -69.4 +/- 13.7, LOS: 0.0 %, DrawRatio: 21.1 %
```

### 2.3

64 bit: 4095 bytes (+6)

MD5: 27319330e722d8a2483b474c823c4ca9

NMP static eval check

```py
info depth 1 score cp 23 time 0 nodes 20 pv b1c3
info depth 2 score cp 16 time 1 nodes 85 nps 85000 pv b1c3
info depth 3 score cp 22 time 1 nodes 341 nps 341000 pv b1c3
info depth 4 score cp 16 time 1 nodes 945 nps 945000 pv b1c3
info depth 5 score cp -1 time 2 nodes 2922 nps 1461000 pv b1c3
info depth 6 score cp 11 time 3 nodes 6898 nps 2299333 pv b1c3
info depth 7 score cp 7 time 7 nodes 20233 nps 2890428 pv b1c3
info depth 8 score cp 13 time 13 nodes 37491 nps 2883923 pv b1c3
info depth 9 score cp 8 time 35 nodes 99455 nps 2841571 pv b1c3
info depth 10 score cp 14 time 54 nodes 161141 nps 2984092 pv b1c3
info depth 11 score cp 12 time 120 nodes 389755 nps 3247958 pv b1c3
info depth 12 score cp 15 time 239 nodes 818011 nps 3422640 pv b1c3
info depth 13 score cp 9 time 515 nodes 1813288 nps 3520947 pv b1c3
info depth 14 score cp 6 time 1040 nodes 3729833 nps 3586377 pv b1c3
info depth 15 score cp 13 time 2894 nodes 10603394 nps 3663923 pv b1c3
info depth 16 score cp 16 time 5746 nodes 21113325 nps 3674438 pv e2e4
bestmove e2e4
21113325 nodes 3674438 nps
```

```py
Elo   | 15.16 +- 8.09 (95%)
SPRT  | 10.0+0.10s Threads=1 Hash=1MB
LLR   | 2.97 (-2.94, 2.94) [0.00, 5.00]
Games | N: 4152 W: 1438 L: 1257 D: 1457
Penta | [167, 415, 773, 512, 209]
https://gedas.pythonanywhere.com/test/625/
```

### 2.4

64 bit: 4095 bytes (=)

MD5: 22fe27dd319d6e7d0af9b67deed33e3f

NMP from depth 2

```py
info depth 1 score cp 23 time 0 nodes 20 pv b1c3
info depth 2 score cp 16 time 0 nodes 85 pv b1c3
info depth 3 score cp 22 time 1 nodes 341 nps 341000 pv b1c3
info depth 4 score cp 16 time 1 nodes 840 nps 840000 pv b1c3
info depth 5 score cp -1 time 1 nodes 2798 nps 2798000 pv b1c3
info depth 6 score cp 11 time 2 nodes 6335 nps 3167500 pv b1c3
info depth 7 score cp 7 time 7 nodes 18029 nps 2575571 pv b1c3
info depth 8 score cp 13 time 12 nodes 31415 nps 2617916 pv b1c3
info depth 9 score cp 12 time 28 nodes 74591 nps 2663964 pv b1c3
info depth 10 score cp 16 time 42 nodes 114324 nps 2722000 pv b1c3
info depth 11 score cp 11 time 89 nodes 273447 nps 3072438 pv b1c3
info depth 12 score cp 11 time 154 nodes 504216 nps 3274129 pv b1c3
info depth 13 score cp 9 time 419 nodes 1449090 nps 3458448 pv b1c3
info depth 14 score cp 10 time 771 nodes 2716000 nps 3522697 pv b1c3
info depth 15 score cp 17 time 2096 nodes 7491325 nps 3574105 pv e2e4
info depth 16 score cp 14 time 3264 nodes 11628117 nps 3562535 pv e2e4
bestmove e2e4
11628117 nodes 3562535 nps
```

```py
Elo   | 19.90 +- 9.31 (95%)
SPRT  | 10.0+0.10s Threads=1 Hash=1MB
LLR   | 2.95 (-2.94, 2.94) [0.00, 5.00]
Games | N: 2902 W: 1037 L: 871 D: 994
Penta | [84, 306, 568, 346, 147]
https://gedas.pythonanywhere.com/test/627/
```

```py
Score of 4k.c-2.4 vs 4ku-1.1: 502 - 683 - 315  [0.440] 1500
...      4k.c-2.4 playing White: 308 - 270 - 173  [0.525] 751
...      4k.c-2.4 playing Black: 194 - 413 - 142  [0.354] 749
...      White vs Black: 721 - 464 - 315  [0.586] 1500
Elo difference: -42.1 +/- 15.7, LOS: 0.0 %, DrawRatio: 21.0 %
```

### 2.5

64 bit: 4025 bytes (-70)

MD5: 48ab37d514b2e89e2b3805b66e702526

Smaller loader

Thanks to **ONE_RANDOM_HUMAN**

### 2.6

64 bit: 4010 bytes (-15)

MD5: fd3faf3520feff9b2576a68865912679

Even smaller loader

Thanks to **ONE_RANDOM_HUMAN**

### 2.7

64 bit: 4010 bytes (=)

MD5: 68dd5fe95483045c38c626012069f7ee

Allow repetition detection after null move

```py
info depth 1 score cp 23 time 0 nodes 20 pv b1c3
info depth 2 score cp 16 time 0 nodes 85 pv b1c3
info depth 3 score cp 22 time 0 nodes 341 pv b1c3
info depth 4 score cp 16 time 1 nodes 840 nps 840000 pv b1c3
info depth 5 score cp -1 time 1 nodes 2798 nps 2798000 pv b1c3
info depth 6 score cp 11 time 3 nodes 6335 nps 2111666 pv b1c3
info depth 7 score cp 7 time 7 nodes 18029 nps 2575571 pv b1c3
info depth 8 score cp 13 time 13 nodes 31415 nps 2416538 pv b1c3
info depth 9 score cp 12 time 31 nodes 74591 nps 2406161 pv b1c3
info depth 10 score cp 16 time 47 nodes 114324 nps 2432425 pv b1c3
info depth 11 score cp 11 time 99 nodes 273447 nps 2762090 pv b1c3
info depth 12 score cp 11 time 169 nodes 504216 nps 2983526 pv b1c3
info depth 13 score cp 9 time 447 nodes 1449090 nps 3241812 pv b1c3
info depth 14 score cp 10 time 828 nodes 2716000 nps 3280193 pv b1c3
info depth 15 score cp 17 time 2280 nodes 7491325 nps 3285668 pv e2e4
info depth 16 score cp 14 time 3517 nodes 11628117 nps 3306260 pv e2e4
bestmove e2e4
11628117 nodes 3306260 nps
```

```py
Elo   | 6.42 +- 6.17 (95%)
SPRT  | 10.0+0.10s Threads=1 Hash=1MB
LLR   | 2.98 (-2.94, 2.94) [-5.00, 0.00]
Games | N: 6280 W: 2037 L: 1921 D: 2322
Penta | [216, 660, 1307, 706, 251]
https://gedas.pythonanywhere.com/test/636/
```

### 2.8

64 bit: 3985 bytes (-25)

MD5: a3f0c7de943b5f5f55754237321900d5

Move history heuristic table to global variable

Thanks to **cj5716**

### 2.9

64 bit: 4039 bytes (+54)

MD5: 1706777a95a5471e92525defdeb05851

History malus

```py
info depth 1 score cp 23 time 0 nodes 20 pv b1c3
info depth 2 score cp 16 time 0 nodes 85 pv b1c3
info depth 3 score cp 22 time 0 nodes 290 pv b1c3
info depth 4 score cp 16 time 0 nodes 778 pv b1c3
info depth 5 score cp -1 time 1 nodes 2592 nps 2592000 pv b1c3
info depth 6 score cp 11 time 3 nodes 8830 nps 2943333 pv f2f4
info depth 7 score cp 5 time 8 nodes 21284 nps 2660500 pv g1f3
info depth 8 score cp 16 time 11 nodes 28972 nps 2633818 pv g1f3
info depth 9 score cp 15 time 32 nodes 80268 nps 2508375 pv b1c3
info depth 10 score cp 16 time 50 nodes 131164 nps 2623280 pv b1c3
info depth 11 score cp 8 time 118 nodes 345125 nps 2924788 pv b1c3
info depth 12 score cp 15 time 231 nodes 746529 nps 3231727 pv b1c3
info depth 13 score cp 6 time 516 nodes 1758204 nps 3407372 pv b1c3
info depth 14 score cp 12 time 927 nodes 3228737 nps 3482995 pv b1c3
info depth 15 score cp 14 time 2315 nodes 8271934 nps 3573189 pv b1c3
info depth 16 score cp 16 time 5113 nodes 18139884 nps 3547796 pv e2e4
bestmove e2e4
18139884 nodes 3547796 nps
```

```py
Elo   | 24.19 +- 10.51 (95%)
SPRT  | 10.0+0.10s Threads=1 Hash=1MB
LLR   | 2.96 (-2.94, 2.94) [0.00, 5.00]
Games | N: 2446 W: 860 L: 690 D: 896
Penta | [81, 249, 448, 309, 136]
https://gedas.pythonanywhere.com/test/648/
```

```py
Elo   | 44.97 +- 14.49 (95%)
SPRT  | 60.0+0.60s Threads=1 Hash=1MB
LLR   | 2.95 (-2.94, 2.94) [0.00, 5.00]
Games | N: 1282 W: 486 L: 321 D: 475
Penta | [37, 101, 244, 178, 81]
https://gedas.pythonanywhere.com/test/649/
```

### 2.10

64 bit: 4034 bytes (-5)

MD5: fd7db885ed6273aa6a23424ed83e55a3

Smaller time management

```py
info depth 1 score cp 23 time 0 nodes 20 pv b1c3
info depth 2 score cp 16 time 0 nodes 85 pv b1c3
info depth 3 score cp 22 time 1 nodes 214 nps 214000 pv b1c3
info depth 4 score cp 16 time 1 nodes 434 nps 434000 pv b1c3
info depth 5 score cp -1 time 1 nodes 1345 nps 1345000 pv b1c3
info depth 6 score cp 11 time 3 nodes 3744 nps 1248000 pv f2f4
info depth 7 score cp 5 time 5 nodes 7828 nps 1565600 pv g1f3
info depth 8 score cp 15 time 8 nodes 11531 nps 1441375 pv g1f3
info depth 9 score cp 15 time 13 nodes 18418 nps 1416769 pv g1f3
info depth 10 score cp 14 time 27 nodes 45602 nps 1688962 pv g1f3
info depth 11 score cp 10 time 60 nodes 125723 nps 2095383 pv g1f3
info depth 12 score cp 15 time 121 nodes 307816 nps 2543933 pv g1f3
info depth 13 score cp 10 time 377 nodes 1109697 nps 2943493 pv e2e4
info depth 14 score cp 16 time 660 nodes 2023543 nps 3065974 pv e2e4
info depth 15 score cp 16 time 1153 nodes 3644634 nps 3161000 pv e2e4
info depth 16 score cp 12 time 2068 nodes 6630121 nps 3206054 pv e2e4
info depth 17 score cp 12 time 3707 nodes 12035438 nps 3246678 pv e2e4
bestmove e2e4
12035438 nodes 3246678 nps
```

10+0.1:
```py
Score of 4k.c-2.10 vs 4ku-1.1: 1034 - 1346 - 620  [0.448] 3000
...      4k.c-2.10 playing White: 631 - 521 - 348  [0.537] 1500
...      4k.c-2.10 playing Black: 403 - 825 - 272  [0.359] 1500
...      White vs Black: 1456 - 924 - 620  [0.589] 3000
Elo difference: -36.3 +/- 11.1, LOS: 0.0 %, DrawRatio: 20.7 %
```

60+0.6:
```py
Score of 4k.c-2.10 vs 4ku-1.1: 513 - 615 - 372  [0.466] 1500
...      4k.c-2.10 playing White: 327 - 229 - 194  [0.565] 750
...      4k.c-2.10 playing Black: 186 - 386 - 178  [0.367] 750
...      White vs Black: 713 - 415 - 372  [0.599] 1500
Elo difference: -23.7 +/- 15.3, LOS: 0.1 %, DrawRatio: 24.8 %
```

### 2.11

64 bit: 4092 bytes (+58)

MD5: 58ee831c562065d98fbaff7c2cdb0ac8

Late move pruning

```py
info depth 1 score cp 23 time 0 nodes 20 pv b1c3
info depth 2 score cp 16 time 0 nodes 85 pv b1c3
info depth 3 score cp 22 time 0 nodes 214 pv b1c3
info depth 4 score cp 16 time 1 nodes 434 nps 434000 pv b1c3
info depth 5 score cp -1 time 1 nodes 1345 nps 1345000 pv b1c3
info depth 6 score cp 11 time 3 nodes 3744 nps 1248000 pv f2f4
info depth 7 score cp 5 time 5 nodes 7828 nps 1565600 pv g1f3
info depth 8 score cp 15 time 8 nodes 11531 nps 1441375 pv g1f3
info depth 9 score cp 15 time 13 nodes 18418 nps 1416769 pv g1f3
info depth 10 score cp 14 time 27 nodes 45602 nps 1688962 pv g1f3
info depth 11 score cp 10 time 62 nodes 125723 nps 2027790 pv g1f3
info depth 12 score cp 15 time 126 nodes 307816 nps 2442984 pv g1f3
info depth 13 score cp 10 time 405 nodes 1109697 nps 2739992 pv e2e4
info depth 14 score cp 16 time 709 nodes 2023543 nps 2854080 pv e2e4
info depth 15 score cp 16 time 1230 nodes 3644634 nps 2963117 pv e2e4
info depth 16 score cp 12 time 2188 nodes 6630121 nps 3030219 pv e2e4
info depth 17 score cp 12 time 3912 nodes 12035438 nps 3076543 pv e2e4
bestmove e2e4
12035438 nodes 3076543 nps
```

```py
Elo   | 52.67 +- 15.80 (95%)
SPRT  | 10.0+0.10s Threads=1 Hash=1MB
LLR   | 3.01 (-2.94, 2.94) [0.00, 5.00]
Games | N: 1150 W: 475 L: 302 D: 373
Penta | [34, 89, 204, 166, 82]
https://gedas.pythonanywhere.com/test/658/
```

```py
Elo   | 35.48 +- 12.79 (95%)
SPRT  | 60.0+0.60s Threads=1 Hash=1MB
LLR   | 2.95 (-2.94, 2.94) [0.00, 5.00]
Games | N: 1582 W: 548 L: 387 D: 647
Penta | [37, 163, 282, 220, 89]
https://gedas.pythonanywhere.com/test/659/
```

### 3.0

Equivalent to 2.11

Last version with LZ4 compression

### 3.1

64 bit: 3870 bytes (-222)

MD5: e708a814053274a81c49ae7dfb67307c

aPLib compression

### 3.2

64 bit: 3859 bytes (-11)

MD5: d49e37d2f507e0a7605ee349359234ae

Smaller aPLib compression

### 3.3

64 bit: 3909 bytes (+50)

MD5: 2558390741a13d957482ab7277efc71b

* History gravity
* Capture history

```py
info depth 1 score cp 23 time 0 nodes 20 pv b1c3
info depth 2 score cp 16 time 0 nodes 85 pv b1c3
info depth 3 score cp 22 time 0 nodes 214 pv b1c3
info depth 4 score cp 16 time 1 nodes 434 nps 434000 pv b1c3
info depth 5 score cp -1 time 1 nodes 1333 nps 1333000 pv b1c3
info depth 6 score cp 11 time 3 nodes 3736 nps 1245333 pv f2f4
info depth 7 score cp 5 time 5 nodes 7338 nps 1467600 pv g1f3
info depth 8 score cp 17 time 7 nodes 10551 nps 1507285 pv g1f3
info depth 9 score cp 10 time 16 nodes 24958 nps 1559875 pv g1f3
info depth 10 score cp 14 time 35 nodes 62772 nps 1793485 pv g1f3
info depth 11 score cp 15 time 59 nodes 125839 nps 2132864 pv g1f3
info depth 12 score cp 15 time 120 nodes 315124 nps 2626033 pv g1f3
info depth 13 score cp 10 time 260 nodes 767877 nps 2953373 pv g1f3
info depth 14 score cp 12 time 473 nodes 1460577 nps 3087900 pv g1f3
info depth 15 score cp 12 time 1448 nodes 4665589 nps 3222091 pv e2e4
info depth 16 score cp 16 time 2142 nodes 6935377 nps 3237804 pv e2e4
info depth 17 score cp 8 time 5476 nodes 18115444 nps 3308152 pv e2e4
bestmove e2e4
18115444 nodes 3308152 nps
```

```py
Elo   | 22.80 +- 10.15 (95%)
SPRT  | 10.0+0.10s Threads=1 Hash=1MB
LLR   | 2.95 (-2.94, 2.94) [0.00, 5.00]
Games | N: 2564 W: 884 L: 716 D: 964
Penta | [73, 292, 446, 336, 135]
https://gedas.pythonanywhere.com/test/670/
```

```py
Elo   | 44.33 +- 13.79 (95%)
SPRT  | 60.0+0.60s Threads=1 Hash=1MB
LLR   | 2.95 (-2.94, 2.94) [0.00, 5.00]
Games | N: 1182 W: 413 L: 263 D: 506
Penta | [18, 108, 226, 184, 55]
https://gedas.pythonanywhere.com/test/671/
```

### 3.4

64 bit: 3905 bytes (-4)

MD5: b198ac9f084af478a4f11f37f812ae0f

Smaller history heuristic

### 3.5

64 bit: 3944 bytes (+39)

MD5: 127fb3523ffb459ac90c50deb8fbd754

Fail-soft main search

```py
info depth 1 score cp 23 time 0 nodes 20 pv b1c3
info depth 2 score cp 16 time 0 nodes 85 pv b1c3
info depth 3 score cp 22 time 0 nodes 214 pv b1c3
info depth 4 score cp 16 time 0 nodes 433 pv b1c3
info depth 5 score cp -1 time 1 nodes 1308 nps 1308000 pv b1c3
info depth 6 score cp 11 time 2 nodes 3519 nps 1759500 pv f2f4
info depth 7 score cp 5 time 5 nodes 7010 nps 1402000 pv g1f3
info depth 8 score cp 17 time 7 nodes 10226 nps 1460857 pv g1f3
info depth 9 score cp 8 time 16 nodes 23097 nps 1443562 pv g1f3
info depth 10 score cp 17 time 23 nodes 36140 nps 1571304 pv g1f3
info depth 11 score cp 10 time 88 nodes 198211 nps 2252397 pv b1c3
info depth 12 score cp 15 time 151 nodes 388998 nps 2576145 pv b1c3
info depth 13 score cp 10 time 269 nodes 743801 nps 2765059 pv b1c3
info depth 14 score cp 18 time 574 nodes 1671148 nps 2911407 pv b1c3
info depth 15 score cp 17 time 945 nodes 2799378 nps 2962304 pv b1c3
info depth 16 score cp 9 time 1937 nodes 5903881 nps 3047950 pv b1c3
info depth 17 score cp 12 time 4390 nodes 13642699 nps 3107676 pv b1c3
bestmove b1c3
13642699 nodes 3107676 nps
```

```py
Elo   | 28.90 +- 11.33 (95%)
SPRT  | 10.0+0.10s Threads=1 Hash=1MB
LLR   | 3.00 (-2.94, 2.94) [0.00, 5.00]
Games | N: 2000 W: 700 L: 534 D: 766
Penta | [58, 193, 379, 265, 105]
https://gedas.pythonanywhere.com/test/681/
```

```py
Elo   | 32.06 +- 11.82 (95%)
SPRT  | 60.0+0.60s Threads=1 Hash=1MB
LLR   | 2.96 (-2.94, 2.94) [0.00, 5.00]
Games | N: 1728 W: 598 L: 439 D: 691
Penta | [45, 162, 322, 259, 76]
https://gedas.pythonanywhere.com/test/683/
```

### 3.6

64 bit: 3979 bytes (+35)

MD5: 7cc7afd9f983bf4a2124e815d3f46c7d

Static eval TT adjustment

```py
info depth 1 score cp 23 time 1 nodes 20 nps 20000 pv b1c3
info depth 2 score cp 16 time 1 nodes 85 nps 85000 pv b1c3
info depth 3 score cp 22 time 1 nodes 214 nps 214000 pv b1c3
info depth 4 score cp 16 time 1 nodes 431 nps 431000 pv b1c3
info depth 5 score cp -1 time 2 nodes 1302 nps 651000 pv b1c3
info depth 6 score cp 11 time 4 nodes 3499 nps 874750 pv f2f4
info depth 7 score cp 5 time 6 nodes 6947 nps 1157833 pv g1f3
info depth 8 score cp 17 time 10 nodes 10056 nps 1005600 pv g1f3
info depth 9 score cp 2 time 19 nodes 21370 nps 1124736 pv g1f3
info depth 10 score cp 14 time 61 nodes 101154 nps 1658262 pv b1c3
info depth 11 score cp 4 time 103 nodes 218684 nps 2123145 pv b1c3
info depth 12 score cp 15 time 148 nodes 349477 nps 2361331 pv b1c3
info depth 13 score cp 6 time 283 nodes 767031 nps 2710356 pv b1c3
info depth 14 score cp 17 time 582 nodes 1701207 nps 2923036 pv b1c3
info depth 15 score cp 17 time 1776 nodes 5484927 nps 3088359 pv e2e4
info depth 16 score cp 16 time 2394 nodes 7420043 nps 3099433 pv e2e4
info depth 17 score cp 12 time 4894 nodes 15230222 nps 3112019 pv e2e4
bestmove e2e4
15230222 nodes 3112019 nps
```

```py
Elo   | 32.72 +- 12.21 (95%)
SPRT  | 10.0+0.10s Threads=1 Hash=1MB
LLR   | 2.95 (-2.94, 2.94) [0.00, 5.00]
Games | N: 1736 W: 635 L: 472 D: 629
Penta | [52, 154, 336, 231, 95]
https://gedas.pythonanywhere.com/test/688/
```

```py
Elo   | 18.82 +- 8.82 (95%)
SPRT  | 60.0+0.60s Threads=1 Hash=1MB
LLR   | 2.96 (-2.94, 2.94) [0.00, 5.00]
Games | N: 2938 W: 952 L: 793 D: 1193
Penta | [67, 331, 561, 396, 114]
https://gedas.pythonanywhere.com/test/689/
```

10+0.1:
```py
Score of 4k.c-3.6 vs 4ku-1.1: 2944 - 2707 - 1349  [0.517] 7000
...      4k.c-3.6 playing White: 1748 - 1023 - 729  [0.604] 3500
...      4k.c-3.6 playing Black: 1196 - 1684 - 620  [0.430] 3500
...      White vs Black: 3432 - 2219 - 1349  [0.587] 7000
Elo difference: 11.8 +/- 7.3, LOS: 99.9 %, DrawRatio: 19.3 %
```

60+0.6:
```py
Score of 4k.c-3.6 vs 4ku-1.1: 439 - 309 - 252  [0.565] 1000
...      4k.c-3.6 playing White: 271 - 104 - 125  [0.667] 500
...      4k.c-3.6 playing Black: 168 - 205 - 127  [0.463] 500
...      White vs Black: 476 - 272 - 252  [0.602] 1000
Elo difference: 45.4 +/- 18.7, LOS: 100.0 %, DrawRatio: 25.2 %
```

### 3.7

64 bit: 3978 bytes (-1)

MD5: 4f6993647f78b4909da4f7df1e9d1d6e

LMP base moves = 2

```py
info depth 1 score cp 23 time 1 nodes 20 nps 20000 pv b1c3
info depth 2 score cp 16 time 1 nodes 85 nps 85000 pv b1c3
info depth 3 score cp 22 time 1 nodes 208 nps 208000 pv b1c3
info depth 4 score cp 16 time 1 nodes 409 nps 409000 pv b1c3
info depth 5 score cp -1 time 2 nodes 1225 nps 612500 pv b1c3
info depth 6 score cp 11 time 3 nodes 3176 nps 1058666 pv f2f4
info depth 7 score cp 5 time 5 nodes 6320 nps 1264000 pv g1f3
info depth 8 score cp 16 time 9 nodes 10381 nps 1153444 pv g1f3
info depth 9 score cp 7 time 20 nodes 25994 nps 1299700 pv g1f3
info depth 10 score cp 16 time 34 nodes 50985 nps 1499558 pv g1f3
info depth 11 score cp 12 time 100 nodes 221352 nps 2213520 pv e2e4
info depth 12 score cp 13 time 183 nodes 469693 nps 2566628 pv g1f3
info depth 13 score cp 10 time 379 nodes 1038059 nps 2738941 pv e2e4
info depth 14 score cp 13 time 689 nodes 1978028 nps 2870867 pv e2e4
info depth 15 score cp 16 time 1130 nodes 3265950 nps 2890221 pv e2e4
info depth 16 score cp 16 time 1849 nodes 5443121 nps 2943818 pv e2e4
info depth 17 score cp 16 time 3404 nodes 10146672 nps 2980808 pv e2e4
bestmove e2e4
10146672 nodes 2980808 nps
```

```py
Elo   | 4.56 +- 3.60 (95%)
SPRT  | 10.0+0.10s Threads=1 Hash=1MB
LLR   | 2.98 (-2.94, 2.94) [0.00, 5.00]
Games | N: 19954 W: 6408 L: 6146 D: 7400
Penta | [773, 2262, 3733, 2348, 861]
https://gedas.pythonanywhere.com/test/694/
```

```py
Elo   | 3.59 +- 5.72 (95%)
SPRT  | 60.0+0.60s Threads=1 Hash=1MB
LLR   | 0.76 (-2.94, 2.94) [0.00, 5.00]
Games | N: 6964 W: 2098 L: 2026 D: 2840
Penta | [212, 805, 1397, 835, 233]
https://gedas.pythonanywhere.com/test/695/
```

### 3.8

64 bit: 3985 bytes (+7)

MD5: b882ca31f765fa8cf82525d05dc0ee0f

SPSA tune

```py
info depth 1 score cp 23 time 0 nodes 20 pv b1c3
info depth 2 score cp 16 time 0 nodes 85 pv b1c3
info depth 3 score cp 22 time 0 nodes 195 pv b1c3
info depth 4 score cp 16 time 0 nodes 368 pv b1c3
info depth 5 score cp -1 time 1 nodes 1158 nps 1158000 pv b1c3
info depth 6 score cp 11 time 3 nodes 3554 nps 1184666 pv f2f4
info depth 7 score cp 6 time 7 nodes 8416 nps 1202285 pv b1c3
info depth 8 score cp 15 time 11 nodes 13576 nps 1234181 pv b1c3
info depth 9 score cp 5 time 31 nodes 42710 nps 1377741 pv b1c3
info depth 10 score cp 14 time 52 nodes 88856 nps 1708769 pv b1c3
info depth 11 score cp 5 time 111 nodes 258609 nps 2329810 pv b1c3
info depth 12 score cp 11 time 170 nodes 428862 nps 2522717 pv b1c3
info depth 13 score cp 8 time 304 nodes 836736 nps 2752421 pv b1c3
info depth 14 score cp 18 time 638 nodes 1857867 nps 2912017 pv b1c3
info depth 15 score cp 17 time 1553 nodes 4628330 nps 2980251 pv e2e4
info depth 16 score cp 16 time 2353 nodes 7083909 nps 3010586 pv e2e4
info depth 17 score cp 16 time 3140 nodes 9428184 nps 3002606 pv e2e4
bestmove e2e4
9428184 nodes 3002606 nps
```

```py
Elo   | 10.76 +- 6.52 (95%)
SPRT  | 10.0+0.10s Threads=1 Hash=1MB
LLR   | 3.02 (-2.94, 2.94) [0.00, 5.00]
Games | N: 6200 W: 2041 L: 1849 D: 2310
Penta | [246, 652, 1141, 786, 275]
https://gedas.pythonanywhere.com/test/708/
```

```py
Elo   | 11.30 +- 6.61 (95%)
SPRT  | 60.0+0.60s Threads=1 Hash=1MB
LLR   | 2.95 (-2.94, 2.94) [0.00, 5.00]
Games | N: 5260 W: 1612 L: 1441 D: 2207
Penta | [152, 571, 1051, 666, 190]
https://gedas.pythonanywhere.com/test/709/
```

### 3.9

64 bit: 3982 bytes (-3)

MD5: d3d8a11c59a07a033d0a46e34de054f1

SPSA tune

Bench same as before

```py
Elo   | 20.77 +- 9.51 (95%)
SPRT  | 10.0+0.10s Threads=1 Hash=1MB
LLR   | 3.01 (-2.94, 2.94) [0.00, 5.00]
Games | N: 2898 W: 1008 L: 835 D: 1055
Penta | [101, 286, 545, 373, 144]
https://gedas.pythonanywhere.com/test/714/
```

```py
Elo   | 9.98 +- 6.12 (95%)
SPRT  | 60.0+0.60s Threads=1 Hash=1MB
LLR   | 3.00 (-2.94, 2.94) [0.00, 5.00]
Games | N: 6304 W: 1955 L: 1774 D: 2575
Penta | [191, 698, 1247, 771, 245]
https://gedas.pythonanywhere.com/test/715/
```

10+0.1:
```py
Score of 4k.c-3.9 vs 4ku-1.1: 1107 - 899 - 494  [0.542] 2500
...      4k.c-3.9 playing White: 663 - 337 - 250  [0.630] 1250
...      4k.c-3.9 playing Black: 444 - 562 - 244  [0.453] 1250
...      White vs Black: 1225 - 781 - 494  [0.589] 2500
Elo difference: 29.0 +/- 12.2, LOS: 100.0 %, DrawRatio: 19.8 %
```

### 3.10

64 bit: 4049 bytes (+67)

MD5: ab4efb9fed054a499d2eebcf7d6666d5

Mobility evaluation

```py
info depth 1 score cp 26 time 0 nodes 20 pv b1c3
info depth 2 score cp 16 time 0 nodes 94 pv b1c3
info depth 3 score cp 26 time 0 nodes 217 pv b1c3
info depth 4 score cp 16 time 1 nodes 472 nps 472000 pv b1c3
info depth 5 score cp 3 time 1 nodes 1213 nps 1213000 pv b1c3
info depth 6 score cp 16 time 2 nodes 2236 nps 1118000 pv b1c3
info depth 7 score cp 17 time 5 nodes 4855 nps 971000 pv b1c3
info depth 8 score cp 16 time 9 nodes 10612 nps 1179111 pv b1c3
info depth 9 score cp 3 time 20 nodes 26289 nps 1314450 pv b1c3
info depth 10 score cp 15 time 44 nodes 71528 nps 1625636 pv b1c3
info depth 11 score cp 15 time 66 nodes 119980 nps 1817878 pv b1c3
info depth 12 score cp 23 time 122 nodes 264102 nps 2164770 pv b1c3
info depth 13 score cp 13 time 256 nodes 594348 nps 2321671 pv b1c3
info depth 14 score cp 16 time 619 nodes 1504351 nps 2430292 pv e2e4
info depth 15 score cp 8 time 1211 nodes 3114188 nps 2571583 pv b1c3
info depth 16 score cp 10 time 1800 nodes 4739683 nps 2633157 pv b1c3
info depth 17 score cp 15 time 5001 nodes 13642487 nps 2727951 pv b1c3
bestmove b1c3
13642487 nodes 2727951 nps
```

```py
Elo   | 18.09 +- 8.92 (95%)
SPRT  | 10.0+0.10s Threads=1 Hash=1MB
LLR   | 2.96 (-2.94, 2.94) [0.00, 5.00]
Games | N: 3344 W: 1158 L: 984 D: 1202
Penta | [126, 323, 645, 407, 171]
https://gedas.pythonanywhere.com/test/719/
```

```py
Elo   | 34.73 +- 12.31 (95%)
SPRT  | 60.0+0.60s Threads=1 Hash=1MB
LLR   | 2.95 (-2.94, 2.94) [0.00, 5.00]
Games | N: 1566 W: 563 L: 407 D: 596
Penta | [38, 139, 308, 225, 73]
https://gedas.pythonanywhere.com/test/721/
```

### 3.11

64 bit: 4066 bytes (+17)

MD5: 54a1e5ecd84dca905f57bf219f9b9da6

King attacks evaluation

```py
info depth 1 score cp 25 time 0 nodes 20 pv b1c3
info depth 2 score cp 16 time 0 nodes 94 pv b1c3
info depth 3 score cp 24 time 0 nodes 217 pv b1c3
info depth 4 score cp 16 time 1 nodes 460 nps 460000 pv b1c3
info depth 5 score cp 4 time 1 nodes 1160 nps 1160000 pv b1c3
info depth 6 score cp 16 time 2 nodes 2157 nps 1078500 pv b1c3
info depth 7 score cp 16 time 5 nodes 5034 nps 1006800 pv b1c3
info depth 8 score cp 16 time 12 nodes 11973 nps 997750 pv b1c3
info depth 9 score cp 4 time 36 nodes 46779 nps 1299416 pv b1c3
info depth 10 score cp 16 time 53 nodes 78214 nps 1475735 pv b1c3
info depth 11 score cp 14 time 86 nodes 151884 nps 1766093 pv b1c3
info depth 12 score cp 14 time 121 nodes 233278 nps 1927917 pv b1c3
info depth 13 score cp 13 time 219 nodes 470364 nps 2147780 pv b1c3
info depth 14 score cp 19 time 381 nodes 873074 nps 2291532 pv b1c3
info depth 15 score cp 16 time 1631 nodes 4045807 nps 2480568 pv e2e4
info depth 16 score cp 16 time 2110 nodes 5237943 nps 2482437 pv e2e4
info depth 17 score cp 14 time 4656 nodes 11768756 nps 2527653 pv e2e4
bestmove e2e4
11768756 nodes 2527653 nps
```

```py
Elo   | 20.77 +- 9.57 (95%)
SPRT  | 10.0+0.10s Threads=1 Hash=1MB
LLR   | 2.96 (-2.94, 2.94) [0.00, 5.00]
Games | N: 2830 W: 971 L: 802 D: 1057
Penta | [94, 287, 527, 370, 137]
https://gedas.pythonanywhere.com/test/724/
```

```py
Elo   | 37.14 +- 12.62 (95%)
SPRT  | 60.0+0.60s Threads=1 Hash=1MB
LLR   | 2.96 (-2.94, 2.94) [0.00, 5.00]
Games | N: 1418 W: 498 L: 347 D: 573
Penta | [24, 137, 277, 206, 65]
https://gedas.pythonanywhere.com/test/725/
```

### 3.12

64 bit: 3999 bytes (-67)

MD5: 0ea32c8f9d2541c432422631f9fe119e

Smaller aPLib decompressor

### 3.13

64 bit: 3942 bytes (-57)

MD5: a0af0e6ce401f402790ff424882b2d80

Even smaller aPLib decompressor

```py
Score of 4k.c-3.13 vs 4ku-1.1: 487 - 311 - 202  [0.588] 1000
...      4k.c-3.13 playing White: 294 - 110 - 97  [0.684] 501
...      4k.c-3.13 playing Black: 193 - 201 - 105  [0.492] 499
...      White vs Black: 495 - 303 - 202  [0.596] 1000
Elo difference: 61.8 +/- 19.5, LOS: 100.0 %, DrawRatio: 20.2 %
```

```py
Score of 4k.c-3.13 vs 4ku-1.1: 1122 - 455 - 423  [0.667] 2000
...      4k.c-3.13 playing White: 676 - 132 - 193  [0.772] 1001
...      4k.c-3.13 playing Black: 446 - 323 - 230  [0.562] 999
...      White vs Black: 999 - 578 - 423  [0.605] 2000
Elo difference: 120.5 +/- 14.1, LOS: 100.0 %, DrawRatio: 21.1 %
```

### 3.14

64 bit: 3944 bytes (+2)

MD5: ee8717bdc2dafdbae5d059872b3a575e

No castling in qsearch

```py
info depth 1 score cp 25 time 0 nodes 20 pv b1c3
info depth 2 score cp 16 time 0 nodes 94 pv b1c3
info depth 3 score cp 24 time 0 nodes 217 pv b1c3
info depth 4 score cp 16 time 1 nodes 460 nps 460000 pv b1c3
info depth 5 score cp 4 time 1 nodes 1160 nps 1160000 pv b1c3
info depth 6 score cp 16 time 2 nodes 2156 nps 1078000 pv b1c3
info depth 7 score cp 16 time 5 nodes 5016 nps 1003200 pv b1c3
info depth 8 score cp 16 time 11 nodes 11875 nps 1079545 pv b1c3
info depth 9 score cp 4 time 33 nodes 46233 nps 1401000 pv b1c3
info depth 10 score cp 16 time 50 nodes 77651 nps 1553020 pv b1c3
info depth 11 score cp 17 time 80 nodes 148171 nps 1852137 pv b1c3
info depth 12 score cp 26 time 142 nodes 305777 nps 2153359 pv b1c3
info depth 13 score cp 13 time 262 nodes 615056 nps 2347541 pv b1c3
info depth 14 score cp 14 time 503 nodes 1261022 nps 2507001 pv d2d4
info depth 15 score cp 16 time 1377 nodes 3675472 nps 2669188 pv d2d4
info depth 16 score cp 16 time 1936 nodes 5067242 nps 2617377 pv d2d4
info depth 17 score cp 13 time 5061 nodes 12753237 nps 2519904 pv d2d4
bestmove d2d4
12753237 nodes 2519904 nps
```

```py
Elo   | 2.73 +- 5.40 (95%)
SPRT  | 10.0+0.10s Threads=1 Hash=1MB
LLR   | 0.44 (-2.94, 2.94) [0.00, 5.00]
Games | N: 8148 W: 2537 L: 2473 D: 3138
Penta | [269, 953, 1589, 971, 292]
https://gedas.pythonanywhere.com/test/742/
```

### 3.15

64 bit: 3987 bytes (+43)

MD5: 297c12ceb767149f7ef70182eddc9a83

LMP adjustment based on improving

```py
info depth 1 score cp 25 time 0 nodes 20 pv b1c3
info depth 2 score cp 16 time 0 nodes 101 pv b1c3
info depth 3 score cp 24 time 0 nodes 214 pv b1c3
info depth 4 score cp 16 time 1 nodes 429 nps 429000 pv b1c3
info depth 5 score cp 4 time 1 nodes 974 nps 974000 pv b1c3
info depth 6 score cp 16 time 2 nodes 1670 nps 835000 pv b1c3
info depth 7 score cp 16 time 5 nodes 4190 nps 838000 pv b1c3
info depth 8 score cp 16 time 9 nodes 9152 nps 1016888 pv b1c3
info depth 9 score cp 4 time 22 nodes 24511 nps 1114136 pv b1c3
info depth 10 score cp 11 time 42 nodes 56507 nps 1345404 pv b1c3
info depth 11 score cp 11 time 69 nodes 114481 nps 1659144 pv b1c3
info depth 12 score cp 12 time 115 nodes 218461 nps 1899660 pv b1c3
info depth 13 score cp 7 time 236 nodes 509759 nps 2159995 pv b1c3
info depth 14 score cp 11 time 390 nodes 882068 nps 2261712 pv b1c3
info depth 15 score cp 17 time 924 nodes 2222011 nps 2404773 pv e2e4
info depth 16 score cp 16 time 2049 nodes 5210051 nps 2542728 pv e2e4
info depth 17 score cp 16 time 3248 nodes 8224073 nps 2532042 pv e2e4
info depth 18 score cp 15 time 5723 nodes 14360104 nps 2509191 pv e2e4
bestmove e2e4
14360104 nodes 2509191 nps
```

```py
Elo   | 22.49 +- 9.90 (95%)
SPRT  | 10.0+0.10s Threads=1 Hash=1MB
LLR   | 2.95 (-2.94, 2.94) [0.00, 5.00]
Games | N: 2522 W: 854 L: 691 D: 977
Penta | [76, 245, 498, 324, 118]
https://gedas.pythonanywhere.com/test/754/
```

```py
Elo   | 23.24 +- 9.76 (95%)
SPRT  | 60.0+0.60s Threads=1 Hash=1MB
LLR   | 2.95 (-2.94, 2.94) [0.00, 5.00]
Games | N: 2276 W: 731 L: 579 D: 966
Penta | [50, 224, 472, 308, 84]
https://gedas.pythonanywhere.com/test/755/
```

### 3.16

64 bit: 4000 bytes (+13)

MD5: 789769c6b53f06cb74b7d6b15f69167c

LMR adjustment based on:
* Zero window
* Improving

```py
info depth 1 score cp 25 time 0 nodes 20 pv b1c3
info depth 2 score cp 16 time 0 nodes 101 pv b1c3
info depth 3 score cp 24 time 0 nodes 214 pv b1c3
info depth 4 score cp 16 time 1 nodes 429 nps 429000 pv b1c3
info depth 5 score cp 4 time 1 nodes 974 nps 974000 pv b1c3
info depth 6 score cp 16 time 2 nodes 1670 nps 835000 pv b1c3
info depth 7 score cp 16 time 5 nodes 4190 nps 838000 pv b1c3
info depth 8 score cp 16 time 10 nodes 9152 nps 915200 pv b1c3
info depth 9 score cp 4 time 24 nodes 24511 nps 1021291 pv b1c3
info depth 10 score cp 11 time 46 nodes 56507 nps 1228413 pv b1c3
info depth 11 score cp 11 time 76 nodes 114481 nps 1506328 pv b1c3
info depth 12 score cp 12 time 126 nodes 218461 nps 1733817 pv b1c3
info depth 13 score cp 7 time 260 nodes 509759 nps 1960611 pv b1c3
info depth 14 score cp 11 time 416 nodes 882068 nps 2120355 pv b1c3
info depth 15 score cp 17 time 923 nodes 2222011 nps 2407379 pv e2e4
info depth 16 score cp 16 time 1993 nodes 5210051 nps 2614175 pv e2e4
info depth 17 score cp 16 time 3169 nodes 8224073 nps 2595163 pv e2e4
info depth 18 score cp 15 time 5493 nodes 14360104 nps 2614255 pv e2e4
bestmove e2e4
14360104 nodes 2614255 nps
```

```py
Elo   | 11.74 +- 6.86 (95%)
SPRT  | 10.0+0.10s Threads=1 Hash=1MB
LLR   | 2.95 (-2.94, 2.94) [0.00, 5.00]
Games | N: 5210 W: 1691 L: 1515 D: 2004
Penta | [170, 561, 1014, 643, 217]
https://gedas.pythonanywhere.com/test/761/
```

```py
Elo   | 11.79 +- 6.74 (95%)
SPRT  | 60.0+0.60s Threads=1 Hash=1MB
LLR   | 2.96 (-2.94, 2.94) [0.00, 5.00]
Games | N: 4954 W: 1519 L: 1351 D: 2084
Penta | [129, 560, 974, 642, 172]
https://gedas.pythonanywhere.com/test/763/
```

### 3.17

64 bit: 4002 bytes (+2)

MD5: 3aae135a9593061ef2192c95e6f1d602

3-step PVS

```py
info depth 1 score cp 25 time 0 nodes 20 pv b1c3
info depth 2 score cp 16 time 0 nodes 103 pv b1c3
info depth 3 score cp 24 time 0 nodes 216 pv b1c3
info depth 4 score cp 16 time 1 nodes 431 nps 431000 pv b1c3
info depth 5 score cp 4 time 1 nodes 930 nps 930000 pv b1c3
info depth 6 score cp 16 time 2 nodes 1636 nps 818000 pv b1c3
info depth 7 score cp 16 time 4 nodes 3569 nps 892250 pv b1c3
info depth 8 score cp 16 time 7 nodes 6779 nps 968428 pv b1c3
info depth 9 score cp 4 time 22 nodes 24254 nps 1102454 pv g1f3
info depth 10 score cp 26 time 46 nodes 66107 nps 1437108 pv b1c3
info depth 11 score cp 9 time 68 nodes 109194 nps 1605794 pv b1c3
info depth 12 score cp 12 time 122 nodes 239982 nps 1967065 pv b1c3
info depth 13 score cp 4 time 248 nodes 561681 nps 2264842 pv b1c3
info depth 14 score cp 16 time 383 nodes 907865 nps 2370404 pv b1c3
info depth 15 score cp 12 time 1216 nodes 3101789 nps 2550813 pv e2e4
info depth 16 score cp 13 time 1963 nodes 5070835 nps 2583206 pv e2e4
info depth 17 score cp 13 time 3168 nodes 8116396 nps 2561993 pv e2e4
info depth 18 score cp 25 time 4180 nodes 10682318 nps 2555578 pv e2e4
bestmove e2e4
10682318 nodes 2555578 nps
```

```py
Elo   | 18.08 +- 8.70 (95%)
SPRT  | 10.0+0.10s Threads=1 Hash=1MB
LLR   | 2.97 (-2.94, 2.94) [0.00, 5.00]
Games | N: 3174 W: 1046 L: 881 D: 1247
Penta | [94, 317, 648, 386, 142]
https://gedas.pythonanywhere.com/test/767/
```

```py
Elo   | 13.67 +- 16.59 (95%)
SPRT  | 60.0+0.60s Threads=1 Hash=1MB
LLR   | 0.58 (-2.94, 2.94) [0.00, 5.00]
Games | N: 814 W: 256 L: 224 D: 334
Penta | [21, 89, 161, 109, 27]
https://gedas.pythonanywhere.com/test/769/
```

```py
Score of 4k.c-3.17 vs 4ku-1.1: 1046 - 524 - 430  [0.630] 2000
...      4k.c-3.17 playing White: 620 - 177 - 204  [0.721] 1001
...      4k.c-3.17 playing Black: 426 - 347 - 226  [0.540] 999
...      White vs Black: 967 - 603 - 430  [0.591] 2000
Elo difference: 92.8 +/- 13.8, LOS: 100.0 %, DrawRatio: 21.5 %
```

### 3.18

64 bit: 4092 bytes (+90)

MD5: 935f3951086e2b2e2c7040727ec060e4

Forward futility pruning

```py
info depth 1 score cp 25 time 0 nodes 20 pv b1c3
info depth 2 score cp 16 time 0 nodes 103 pv b1c3
info depth 3 score cp 24 time 0 nodes 216 pv b1c3
info depth 4 score cp 16 time 0 nodes 431 pv b1c3
info depth 5 score cp 4 time 1 nodes 924 nps 924000 pv b1c3
info depth 6 score cp 16 time 2 nodes 1626 nps 813000 pv b1c3
info depth 7 score cp 16 time 4 nodes 3446 nps 861500 pv b1c3
info depth 8 score cp 16 time 10 nodes 9442 nps 944200 pv b1c3
info depth 9 score cp -4 time 19 nodes 19715 nps 1037631 pv b1c3
info depth 10 score cp 15 time 35 nodes 41231 nps 1178028 pv b1c3
info depth 11 score cp 9 time 61 nodes 92394 nps 1514655 pv b1c3
info depth 12 score cp 23 time 97 nodes 166750 nps 1719072 pv b1c3
info depth 13 score cp 18 time 245 nodes 496852 nps 2027967 pv e2e4
info depth 14 score cp 13 time 399 nodes 842345 nps 2111140 pv e2e4
info depth 15 score cp 27 time 716 nodes 1580226 nps 2207019 pv e2e4
info depth 16 score cp 20 time 1212 nodes 2751995 nps 2270622 pv e2e4
info depth 17 score cp 28 time 2271 nodes 5278317 nps 2324225 pv e2e4
info depth 18 score cp 28 time 3565 nodes 8255214 nps 2315628 pv e2e4
info depth 19 score cp 28 time 5871 nodes 13799158 nps 2350393 pv e2e4
bestmove e2e4
13799158 nodes 2350393 nps
```

```py
Elo   | 21.12 +- 9.65 (95%)
SPRT  | 10.0+0.10s Threads=1 Hash=1MB
LLR   | 2.95 (-2.94, 2.94) [0.00, 5.00]
Games | N: 2750 W: 966 L: 799 D: 985
Penta | [80, 306, 486, 373, 130]
https://gedas.pythonanywhere.com/test/779/
```

```py
Elo   | 23.19 +- 9.72 (95%)
SPRT  | 60.0+0.60s Threads=1 Hash=1MB
LLR   | 2.95 (-2.94, 2.94) [0.00, 5.00]
Games | N: 2236 W: 721 L: 572 D: 943
Penta | [39, 236, 464, 295, 84]
https://gedas.pythonanywhere.com/test/781/
```

```py
Score of 4k.c-3.18 vs 4ku-1.1: 1646 - 768 - 586  [0.646] 3000
...      4k.c-3.18 playing White: 956 - 272 - 273  [0.728] 1501
...      4k.c-3.18 playing Black: 690 - 496 - 313  [0.565] 1499
...      White vs Black: 1452 - 962 - 586  [0.582] 3000
Elo difference: 104.7 +/- 11.5, LOS: 100.0 %, DrawRatio: 19.5 %
```

```py
Score of 4k.c-3.18 vs 4ku-2.0: 663 - 888 - 449  [0.444] 2000
...      4k.c-3.18 playing White: 425 - 343 - 232  [0.541] 1000
...      4k.c-3.18 playing Black: 238 - 545 - 217  [0.346] 1000
...      White vs Black: 970 - 581 - 449  [0.597] 2000
Elo difference: -39.3 +/- 13.5, LOS: 0.0 %, DrawRatio: 22.4 %
```

### 3.19

64 bit: 4087 bytes (-5)

MD5: 86f52992aa7c5147e719e9c15a16812f

Allow repetition detection in qsearch

```py
info depth 1 score cp 25 time 0 nodes 20 pv b1c3
info depth 2 score cp 16 time 0 nodes 103 pv b1c3
info depth 3 score cp 24 time 0 nodes 216 pv b1c3
info depth 4 score cp 16 time 1 nodes 431 nps 431000 pv b1c3
info depth 5 score cp 4 time 1 nodes 924 nps 924000 pv b1c3
info depth 6 score cp 16 time 2 nodes 1626 nps 813000 pv b1c3
info depth 7 score cp 16 time 4 nodes 3446 nps 861500 pv b1c3
info depth 8 score cp 16 time 10 nodes 9442 nps 944200 pv b1c3
info depth 9 score cp -4 time 20 nodes 19734 nps 986700 pv b1c3
info depth 10 score cp 15 time 37 nodes 41250 nps 1114864 pv b1c3
info depth 11 score cp 9 time 63 nodes 92413 nps 1466873 pv b1c3
info depth 12 score cp 23 time 99 nodes 166768 nps 1684525 pv b1c3
info depth 13 score cp 18 time 246 nodes 496907 nps 2019947 pv e2e4
info depth 14 score cp 13 time 384 nodes 795868 nps 2072572 pv e2e4
info depth 15 score cp 21 time 691 nodes 1526867 nps 2209648 pv e2e4
info depth 16 score cp 25 time 1237 nodes 2851118 nps 2304864 pv e2e4
info depth 17 score cp 16 time 2367 nodes 5502728 nps 2324768 pv e2e4
info depth 18 score cp 15 time 4368 nodes 10000816 nps 2289564 pv e2e4
info depth 19 score cp 22 time 11013 nodes 27231627 nps 2472680 pv e2e4
bestmove e2e4
27231627 nodes 2472680 nps
```

```py
Elo   | 1.20 +- 3.80 (95%)
SPRT  | 10.0+0.10s Threads=1 Hash=1MB
LLR   | 2.97 (-2.94, 2.94) [-5.00, 0.00]
Games | N: 17330 W: 5359 L: 5299 D: 6672
Penta | [655, 1999, 3312, 2029, 670]
https://gedas.pythonanywhere.com/test/804/
```

```py
Elo   | 3.79 +- 10.77 (95%)
SPRT  | 60.0+0.60s Threads=1 Hash=1MB
LLR   | 0.64 (-2.94, 2.94) [-5.00, 0.00]
Games | N: 1924 W: 579 L: 558 D: 787
Penta | [48, 236, 396, 211, 71]
https://gedas.pythonanywhere.com/test/811/
```

### 3.20

64 bit: 4085 bytes (-2)

MD5: eefaa038313d5b228a0694c4e104558c

No repetition detection after null move

```py
info depth 1 score cp 25 time 0 nodes 20 pv b1c3
info depth 2 score cp 16 time 0 nodes 103 pv b1c3
info depth 3 score cp 24 time 0 nodes 216 pv b1c3
info depth 4 score cp 16 time 0 nodes 431 pv b1c3
info depth 5 score cp 4 time 1 nodes 924 nps 924000 pv b1c3
info depth 6 score cp 16 time 2 nodes 1626 nps 813000 pv b1c3
info depth 7 score cp 16 time 4 nodes 3446 nps 861500 pv b1c3
info depth 8 score cp 16 time 10 nodes 9442 nps 944200 pv b1c3
info depth 9 score cp -4 time 21 nodes 19734 nps 939714 pv b1c3
info depth 10 score cp 15 time 37 nodes 41250 nps 1114864 pv b1c3
info depth 11 score cp 9 time 64 nodes 92413 nps 1443953 pv b1c3
info depth 12 score cp 23 time 100 nodes 166768 nps 1667680 pv b1c3
info depth 13 score cp 10 time 250 nodes 491133 nps 1964532 pv e2e4
info depth 14 score cp 12 time 401 nodes 824459 nps 2056007 pv e2e4
info depth 15 score cp 18 time 672 nodes 1451131 nps 2159421 pv e2e4
info depth 16 score cp 17 time 1411 nodes 3164119 nps 2242465 pv e2e4
info depth 17 score cp 21 time 2505 nodes 5756583 nps 2298037 pv e2e4
info depth 18 score cp 21 time 3899 nodes 9083136 nps 2329606 pv e2e4
info depth 19 score cp 18 time 6395 nodes 15170468 nps 2372238 pv e2e4
bestmove e2e4
15170468 nodes 2372238 nps
```

```py
Elo   | 1.20 +- 3.80 (95%)
SPRT  | 10.0+0.10s Threads=1 Hash=1MB
LLR   | 2.97 (-2.94, 2.94) [-5.00, 0.00]
Games | N: 17330 W: 5359 L: 5299 D: 6672
Penta | [655, 1999, 3312, 2029, 670]
https://gedas.pythonanywhere.com/test/804/
```

```py
Elo   | 3.79 +- 10.77 (95%)
SPRT  | 60.0+0.60s Threads=1 Hash=1MB
LLR   | 0.64 (-2.94, 2.94) [-5.00, 0.00]
Games | N: 1924 W: 579 L: 558 D: 787
Penta | [48, 236, 396, 211, 71]
https://gedas.pythonanywhere.com/test/811/
```

### 4.0

64 bit: 4085 bytes (=)

MD5: eefaa038313d5b228a0694c4e104558c

Functionally equivalent to version 3.20
Last version with the C aPLib decompressor

### 4.1

64 bit: 3897 bytes (-188)

MD5: 4ecc8bd5946fb869a11748ded02af2db

Use assembly version of aPLib decompressor

### 4.2

64 bit: 3887 bytes (-10)

MD5: 7c277f404f964b56c422616eb9121a0d

Don't compute final size in aPLib

### 4.3

64 bit: 3880 bytes (-7)

MD5: 3fe3bf36db6ce87a3e007e5ec974bbc5

Disable >32kb case in aPLib

### 4.4

64 bit: 3875 bytes (-5)

MD5: e9015773d544c846f89bc17b05ea6004

Smaller aPLib decompression

### 4.5

64 bit: 3859 bytes (-16)

MD5: 6d3cc7a1a1036752b581c5bfe6270591

Smaller IIR

### 4.6

64 bit: 3867 bytes (+8)

MD5: fbb81a25bc9f065b3978ad15a658c81d

NMP depth adjust by depth

```py
info depth 1 score cp 25 time 0 nodes 20 pv b1c3
info depth 2 score cp 16 time 0 nodes 103 pv b1c3
info depth 3 score cp 24 time 1 nodes 216 nps 216000 pv b1c3
info depth 4 score cp 16 time 1 nodes 431 nps 431000 pv b1c3
info depth 5 score cp 4 time 1 nodes 924 nps 924000 pv b1c3
info depth 6 score cp 16 time 2 nodes 1626 nps 813000 pv b1c3
info depth 7 score cp 16 time 4 nodes 3446 nps 861500 pv b1c3
info depth 8 score cp 16 time 10 nodes 9444 nps 944400 pv b1c3
info depth 9 score cp -4 time 19 nodes 19710 nps 1037368 pv b1c3
info depth 10 score cp 15 time 38 nodes 45661 nps 1201605 pv b1c3
info depth 11 score cp 3 time 71 nodes 108246 nps 1524591 pv b1c3
info depth 12 score cp 8 time 109 nodes 192265 nps 1763899 pv b1c3
info depth 13 score cp 0 time 268 nodes 577768 nps 2155850 pv d2d4
info depth 14 score cp 16 time 542 nodes 1249764 nps 2305837 pv e2e4
info depth 15 score cp 20 time 785 nodes 1850518 nps 2357347 pv e2e4
info depth 16 score cp 26 time 1172 nodes 2821836 nps 2407709 pv e2e4
info depth 17 score cp 21 time 1914 nodes 4710069 nps 2460851 pv e2e4
info depth 18 score cp 19 time 2864 nodes 7089562 nps 2475405 pv e2e4
info depth 19 score cp 19 time 4247 nodes 10445996 nps 2459617 pv e2e4
bestmove e2e4
10445996 nodes 2459617 nps
```

```py
Elo   | 9.73 +- 6.10 (95%)
SPRT  | 10.0+0.10s Threads=1 Hash=1MB
LLR   | 3.00 (-2.94, 2.94) [0.00, 5.00]
Games | N: 6750 W: 2196 L: 2007 D: 2547
Penta | [235, 737, 1295, 820, 288]
https://gedas.pythonanywhere.com/test/861/
```

```py
Elo   | 8.07 +- 5.29 (95%)
SPRT  | 60.0+0.60s Threads=1 Hash=1MB
LLR   | 2.99 (-2.94, 2.94) [0.00, 5.00]
Games | N: 7794 W: 2392 L: 2211 D: 3191
Penta | [208, 877, 1581, 988, 243]
https://gedas.pythonanywhere.com/test/862/
```

### 4.7

64 bit: 3867 bytes (=)

MD5: f8d8533a707b6e4627daefc05e953e76

LMR move divisor 11

```py
info depth 1 score cp 25 time 1 nodes 20 nps 20000 pv b1c3
info depth 2 score cp 16 time 1 nodes 103 nps 103000 pv b1c3
info depth 3 score cp 24 time 1 nodes 214 nps 214000 pv b1c3
info depth 4 score cp 16 time 1 nodes 420 nps 420000 pv b1c3
info depth 5 score cp 4 time 2 nodes 902 nps 451000 pv b1c3
info depth 6 score cp 16 time 2 nodes 1577 nps 788500 pv b1c3
info depth 7 score cp 16 time 4 nodes 3386 nps 846500 pv b1c3
info depth 8 score cp 16 time 8 nodes 6474 nps 809250 pv b1c3
info depth 9 score cp 4 time 21 nodes 20154 nps 959714 pv b1c3
info depth 10 score cp 23 time 46 nodes 57451 nps 1248934 pv b1c3
info depth 11 score cp 9 time 71 nodes 103591 nps 1459028 pv b1c3
info depth 12 score cp 15 time 113 nodes 194210 nps 1718672 pv b1c3
info depth 13 score cp 11 time 277 nodes 566830 nps 2046317 pv e2e4
info depth 14 score cp 13 time 398 nodes 848084 nps 2130864 pv e2e4
info depth 15 score cp 17 time 583 nodes 1281762 nps 2198562 pv e2e4
info depth 16 score cp 18 time 880 nodes 2009780 nps 2283840 pv e2e4
info depth 17 score cp 15 time 1526 nodes 3634584 nps 2381771 pv e2e4
info depth 18 score cp 14 time 2432 nodes 6062041 nps 2492615 pv e2e4
info depth 19 score cp 15 time 3977 nodes 10168095 nps 2556724 pv e2e4
bestmove e2e4
10168095 nodes 2556724 nps
```

```py
Elo   | 7.25 +- 5.07 (95%)
SPRT  | 10.0+0.10s Threads=1 Hash=1MB
LLR   | 2.96 (-2.94, 2.94) [0.00, 5.00]
Games | N: 9584 W: 3066 L: 2866 D: 3652
Penta | [329, 1060, 1882, 1124, 397]
https://gedas.pythonanywhere.com/test/863/
```

```py
Elo   | 4.02 +- 3.21 (95%)
SPRT  | 60.0+0.60s Threads=1 Hash=1MB
LLR   | 2.96 (-2.94, 2.94) [0.00, 5.00]
Games | N: 22066 W: 6576 L: 6321 D: 9169
Penta | [672, 2539, 4409, 2688, 725]
https://gedas.pythonanywhere.com/test/864/
```

### 4.8

64 bit: 3859 bytes (-8)

MD5: 0af4fc7cf0aa92d88c49552c8ed9defe

Precompute history bonus


### 4.9

64 bit: 3859 bytes (=)

MD5: 8c6cd27785a29f5f1390685ec56780bc

Dedicated puts function

### 4.10

64 bit: 3851 bytes (-8)

MD5: dd7b586f9eed482ae08f22fc43f059a7

Null move pruning fail soft

```py
info depth 1 score cp 25 time 0 nodes 20 pv b1c3
info depth 2 score cp 16 time 0 nodes 103 pv b1c3
info depth 3 score cp 24 time 1 nodes 214 nps 214000 pv b1c3
info depth 4 score cp 16 time 1 nodes 420 nps 420000 pv b1c3
info depth 5 score cp 4 time 1 nodes 902 nps 902000 pv b1c3
info depth 6 score cp 16 time 2 nodes 1577 nps 788500 pv b1c3
info depth 7 score cp 16 time 4 nodes 3386 nps 846500 pv b1c3
info depth 8 score cp 16 time 7 nodes 6474 nps 924857 pv b1c3
info depth 9 score cp 4 time 19 nodes 20075 nps 1056578 pv b1c3
info depth 10 score cp 23 time 42 nodes 57329 nps 1364976 pv b1c3
info depth 11 score cp 9 time 64 nodes 99862 nps 1560343 pv b1c3
info depth 12 score cp 25 time 90 nodes 155228 nps 1724755 pv b1c3
info depth 13 score cp 19 time 330 nodes 735077 nps 2227506 pv e2e4
info depth 14 score cp 13 time 526 nodes 1223716 nps 2326456 pv e2e4
info depth 15 score cp 17 time 694 nodes 1652678 nps 2381380 pv e2e4
info depth 16 score cp 17 time 1054 nodes 2564397 nps 2433014 pv e2e4
info depth 17 score cp 22 time 1572 nodes 3911765 nps 2488400 pv e2e4
info depth 18 score cp 22 time 2350 nodes 5963640 nps 2537719 pv e2e4
info depth 19 score cp 24 time 3459 nodes 8798707 nps 2543714 pv e2e4
bestmove e2e4
8798707 nodes 2543714 nps
```

```py
Elo   | 0.66 +- 3.43 (95%)
SPRT  | 10.0+0.10s Threads=1 Hash=1MB
LLR   | 2.96 (-2.94, 2.94) [-5.00, 0.00]
Games | N: 20954 W: 6393 L: 6353 D: 8208
Penta | [777, 2432, 4015, 2480, 773]
https://gedas.pythonanywhere.com/test/888/
```

```py
Elo   | 5.24 +- 5.66 (95%)
SPRT  | 60.0+0.60s Threads=1 Hash=1MB
LLR   | 2.95 (-2.94, 2.94) [-5.00, 0.00]
Games | N: 7164 W: 2138 L: 2030 D: 2996
Penta | [213, 838, 1402, 886, 243]
https://gedas.pythonanywhere.com/test/890/
```

### 4.11

64 bit: 3843 bytes (-8)

MD5: 7b26ebe7dab289a4f9ddc0e9d4a04664

Smaller pruning code

### 4.12

64 bit: 3835 bytes (-8)

MD5: 3cfc03375747e48a50339d024de80928

No padding for compressed payload

### 4.13

64 bit: 3833 bytes (-2)

MD5: 214a90d70f5d2b8f8c2a1601c5da3cbe

Remove padding fill for loader

10+0.10:
```py
Score of 4k.c-4.13 vs 4ku-2.0: 878 - 1067 - 556  [0.462] 2501
...      4k.c-4.13 playing White: 563 - 403 - 283  [0.564] 1249
...      4k.c-4.13 playing Black: 315 - 664 - 273  [0.361] 1252
...      White vs Black: 1227 - 718 - 556  [0.602] 2501
Elo difference: -26.3 +/- 12.0, LOS: 0.0 %, DrawRatio: 22.2 %
```

60+0.60
```py
Score of 4k.c-4.13 vs 4ku-2.0: 413 - 340 - 247  [0.536] 1000
...      4k.c-4.13 playing White: 276 - 113 - 111  [0.663] 500
...      4k.c-4.13 playing Black: 137 - 227 - 136  [0.410] 500
...      White vs Black: 503 - 250 - 247  [0.626] 1000
Elo difference: 25.4 +/- 18.7, LOS: 99.6 %, DrawRatio: 24.7 %
```

### 4.14

64 bit: 4068 bytes (+235)

MD5: 85561474479af3aa754537b17d837868

* Tapered evaluation
* No forward futility pruning

```py
info depth 1 score cp 25 time 0 nodes 20 pv b1c3
info depth 2 score cp 16 time 0 nodes 103 pv b1c3
info depth 3 score cp 23 time 0 nodes 214 pv b1c3
info depth 4 score cp 16 time 1 nodes 416 nps 416000 pv b1c3
info depth 5 score cp 3 time 1 nodes 1060 nps 1060000 pv b1c3
info depth 6 score cp 16 time 2 nodes 1899 nps 949500 pv b1c3
info depth 7 score cp 13 time 4 nodes 4228 nps 1057000 pv b1c3
info depth 8 score cp 16 time 8 nodes 8625 nps 1078125 pv b1c3
info depth 9 score cp 5 time 22 nodes 23910 nps 1086818 pv c2c4
info depth 10 score cp 13 time 49 nodes 68143 nps 1390673 pv b1c3
info depth 11 score cp 12 time 105 nodes 186273 nps 1774028 pv g1f3
info depth 12 score cp 17 time 169 nodes 327322 nps 1936816 pv g1f3
info depth 13 score cp 10 time 345 nodes 730494 nps 2117373 pv g1f3
info depth 14 score cp 16 time 645 nodes 1424194 nps 2208052 pv g1f3
info depth 15 score cp 16 time 1435 nodes 3248293 nps 2263618 pv c2c4
info depth 16 score cp 27 time 3461 nodes 7892076 nps 2280287 pv e2e4
info depth 17 score cp 20 time 4253 nodes 9679540 nps 2275932 pv e2e4
info depth 18 score cp 24 time 5285 nodes 12098004 nps 2289120 pv e2e4
info depth 19 score cp 32 time 7600 nodes 17509443 nps 2303874 pv e2e4
bestmove e2e4
17509443 nodes 2303874 nps
```

```py
Elo   | 63.48 +- 17.90 (95%)
SPRT  | 10.0+0.10s Threads=1 Hash=1MB
LLR   | 2.96 (-2.94, 2.94) [0.00, 5.00]
Games | N: 974 W: 434 L: 258 D: 282
Penta | [29, 70, 170, 132, 86]
https://gedas.pythonanywhere.com/test/907/
```

```py
Elo   | 85.86 +- 20.03 (95%)
SPRT  | 60.0+0.60s Threads=1 Hash=1MB
LLR   | 2.94 (-2.94, 2.94) [0.00, 5.00]
Games | N: 706 W: 337 L: 166 D: 203
Penta | [13, 35, 139, 100, 66]
https://gedas.pythonanywhere.com/test/908/
```

### 4.15

64 bit: 4053 bytes (-15)

MD5: 227ffa9ef8c6ab06ec17f4ad04f61d5b

Smaller eval structs

### 4.16

64 bit: 4049 bytes (-4)

MD5: cc2de288830538b295222527273f3159

Smaller and faster mobility

```py
Elo   | 18.17 +- 9.58 (95%)
SPRT  | 10.0+0.10s Threads=1 Hash=1MB
LLR   | 2.97 (-2.94, 2.94) [-5.00, 0.00]
Games | N: 2564 W: 846 L: 712 D: 1006
Penta | [70, 265, 517, 321, 109]
https://gedas.pythonanywhere.com/test/910/
```

### 4.17

64 bit: 4041 bytes (-8)

MD5: 22eb2e5999ba3aaff558720dd8e969af

Smaller TT move

```py
Elo   | 8.62 +- 6.92 (95%)
SPRT  | 10.0+0.10s Threads=1 Hash=1MB
LLR   | 2.95 (-2.94, 2.94) [-5.00, 0.00]
Games | N: 4840 W: 1550 L: 1430 D: 1860
Penta | [141, 537, 992, 561, 189]
https://gedas.pythonanywhere.com/test/926/
```

### 4.18

64 bit: 4030 bytes (-11)

MD5: 3206877b9e2672074671abc7703938e6

Smaller is_attacked

```py
Elo   | 0.49 +- 3.24 (95%)
SPRT  | 10.0+0.10s Threads=1 Hash=1MB
LLR   | 2.98 (-2.94, 2.94) [-5.00, 0.00]
Games | N: 22562 W: 6883 L: 6851 D: 8828
Penta | [769, 2624, 4502, 2578, 808]
https://gedas.pythonanywhere.com/test/929/
```

### 4.19

64 bit: 4012 bytes (-18)

MD5: 9c4f65eb5478045303a35d287d1fd2b9

Smaller attack functions

```py
Elo   | 2.14 +- 4.26 (95%)
SPRT  | 10.0+0.10s Threads=1 Hash=1MB
LLR   | 2.96 (-2.94, 2.94) [-5.00, 0.00]
Games | N: 12982 W: 4002 L: 3922 D: 5058
Penta | [438, 1470, 2634, 1472, 477]
https://gedas.pythonanywhere.com/test/932/
```

### 4.20

64 bit: 4090 bytes (+78)

MD5: 03d73379f4135535d2b88a694ea7b7ae

Forward futility pruning

```py
info depth 1 score cp 25 time 0 nodes 20 pv b1c3
info depth 2 score cp 16 time 0 nodes 103 pv b1c3
info depth 3 score cp 23 time 0 nodes 214 pv b1c3
info depth 4 score cp 16 time 0 nodes 416 pv b1c3
info depth 5 score cp 3 time 1 nodes 1008 nps 1008000 pv b1c3
info depth 6 score cp 16 time 1 nodes 1735 nps 1735000 pv b1c3
info depth 7 score cp 13 time 3 nodes 3771 nps 1257000 pv b1c3
info depth 8 score cp 15 time 9 nodes 9468 nps 1052000 pv b1c3
info depth 9 score cp 0 time 24 nodes 25906 nps 1079416 pv b1c3
info depth 10 score cp 18 time 33 nodes 37206 nps 1127454 pv b1c3
info depth 11 score cp 9 time 78 nodes 115980 nps 1486923 pv g1f3
info depth 12 score cp 16 time 122 nodes 207439 nps 1700319 pv g1f3
info depth 13 score cp 11 time 350 nodes 733637 nps 2096105 pv g1f3
info depth 14 score cp 11 time 636 nodes 1401493 nps 2203605 pv g1f3
info depth 15 score cp 11 time 1094 nodes 2445639 nps 2235501 pv c2c4
info depth 16 score cp 23 time 2477 nodes 5705761 nps 2303496 pv e2e4
info depth 17 score cp 25 time 3220 nodes 7476145 nps 2321784 pv e2e4
info depth 18 score cp 22 time 5381 nodes 12624558 nps 2346136 pv e2e4
info depth 19 score cp 31 time 7129 nodes 16711487 nps 2344155 pv e2e4
bestmove e2e4
16711487 nodes 2344155 nps
```

```py
Elo   | 23.25 +- 10.13 (95%)
SPRT  | 10.0+0.10s Threads=1 Hash=1MB
LLR   | 2.97 (-2.94, 2.94) [0.00, 5.00]
Games | N: 2484 W: 856 L: 690 D: 938
Penta | [77, 243, 485, 311, 126]
https://gedas.pythonanywhere.com/test/936/
```

### 4.21

64 bit: 4053 bytes (-37)

MD5: dcec0c6eba7b16e436e17c3bddf7b25f

Automatic code reordering

```py
Elo   | 0.61 +- 3.25 (95%)
SPRT  | 10.0+0.10s Threads=1 Hash=1MB
LLR   | 3.01 (-2.94, 2.94) [-5.00, 0.00]
Games | N: 21480 W: 6479 L: 6441 D: 8560
Penta | [696, 2441, 4429, 2477, 697]
https://gedas.pythonanywhere.com/test/966/
```

```py
Elo   | 2.62 +- 4.33 (95%)
SPRT  | 60.0+0.60s Threads=1 Hash=1MB
LLR   | 2.96 (-2.94, 2.94) [-5.00, 0.00]
Games | N: 11120 W: 3299 L: 3215 D: 4606
Penta | [292, 1255, 2414, 1275, 324]
https://gedas.pythonanywhere.com/test/967/
```

### 4.22

64 bit: 4023 bytes (-30)

MD5: deb3cfaf982bef1a24a77a47a17971ab

More code reordering

```py
Elo   | -3.78 +- 11.17 (95%)
SPRT  | 10.0+0.10s Threads=1 Hash=1MB
LLR   | -0.22 (-2.94, 2.94) [-5.00, 0.00]
Games | N: 1836 W: 536 L: 556 D: 744
Penta | [62, 224, 356, 224, 52]
https://gedas.pythonanywhere.com/test/968/
```

### 4.23

64 bit: 4012 bytes (-11)

MD5: 64fa16aef1b2009a90946bef553c9896

Correlated reordering

```py
Elo   | 1.23 +- 3.68 (95%)
SPRT  | 10.0+0.10s Threads=1 Hash=1MB
LLR   | 2.98 (-2.94, 2.94) [-5.00, 0.00]
Games | N: 16908 W: 5118 L: 5058 D: 6732
Penta | [545, 1947, 3420, 1987, 555]
https://gedas.pythonanywhere.com/test/970/
```

### 4.24

64 bit: 4008 bytes (-4)

MD5: e6e9340b9579831cb3dd96de9c6144ae

Material combining size decrease

### 4.25

64 bit: 3993 bytes (-15)

MD5: 1d382ba32f591ccfb4409c833ae8016d

Multi runs with group shuffling

```py
Elo   | -0.37 +- 2.55 (95%)
SPRT  | 10.0+0.10s Threads=1 Hash=1MB
LLR   | 2.96 (-2.94, 2.94) [-5.00, 0.00]
Games | N: 36060 W: 10917 L: 10955 D: 14188
Penta | [1235, 4178, 7223, 4178, 1216]
https://gedas.pythonanywhere.com/test/978/
```

### 4.26

64 bit: 3970 bytes (-23)

MD5: a43a4dbfd659c2bc2cd0d65e85941ce1

Nested groups

```py
Elo   | 4.58 +- 7.81 (95%)
SPRT  | 10.0+0.10s Threads=1 Hash=1MB
LLR   | 1.43 (-2.94, 2.94) [-5.00, 0.00]
Games | N: 3872 W: 1197 L: 1146 D: 1529
Penta | [138, 413, 776, 478, 131]
https://gedas.pythonanywhere.com/test/982/
```

### 4.27

64 bit: 3968 bytes (-2)

MD5: d89805fc0ae95ed5b197bc81cb0c76a2

More nested groups

### 4.28

64 bit: 4054 bytes (+86)

MD5: f570539145749c0d047c3484f0784e20

Passed pawn evaluation

```py
info depth 1 score cp 25 time 0 nodes 20 pv b1c3
info depth 2 score cp 16 time 0 nodes 103 pv b1c3
info depth 3 score cp 23 time 0 nodes 214 pv b1c3
info depth 4 score cp 16 time 1 nodes 419 nps 419000 pv b1c3
info depth 5 score cp 4 time 1 nodes 1044 nps 1044000 pv b1c3
info depth 6 score cp 16 time 2 nodes 1807 nps 903500 pv b1c3
info depth 7 score cp 13 time 4 nodes 4079 nps 1019750 pv b1c3
info depth 8 score cp 16 time 10 nodes 9238 nps 923800 pv b1c3
info depth 9 score cp 18 time 18 nodes 18636 nps 1035333 pv b1c3
info depth 10 score cp 21 time 29 nodes 34650 nps 1194827 pv b1c3
info depth 11 score cp 3 time 50 nodes 70758 nps 1415160 pv b1c3
info depth 12 score cp 16 time 84 nodes 142368 nps 1694857 pv b1c3
info depth 13 score cp 12 time 232 nodes 497260 nps 2143362 pv d2d4
info depth 14 score cp 12 time 471 nodes 1096641 nps 2328324 pv d2d4
info depth 15 score cp 18 time 774 nodes 1838933 nps 2375882 pv d2d4
info depth 16 score cp 15 time 1333 nodes 3203492 nps 2403219 pv d2d4
info depth 17 score cp 15 time 1996 nodes 4835957 nps 2422824 pv d2d4
info depth 18 score cp 17 time 3513 nodes 8599469 nps 2447898 pv d2d4
info depth 19 score cp 14 time 5571 nodes 13730115 nps 2464569 pv d2d4
bestmove d2d4
13730115 nodes 2464569 nps
```

```py
Elo   | 36.11 +- 12.68 (95%)
SPRT  | 10.0+0.10s Threads=1 Hash=1MB
LLR   | 2.95 (-2.94, 2.94) [0.00, 5.00]
Games | N: 1516 W: 552 L: 395 D: 569
Penta | [35, 136, 306, 199, 82]
https://gedas.pythonanywhere.com/test/992/
```

```py
Elo   | 22.13 +- 9.72 (95%)
SPRT  | 60.0+0.60s Threads=1 Hash=1MB
LLR   | 2.98 (-2.94, 2.94) [0.00, 5.00]
Games | N: 2562 W: 885 L: 722 D: 955
Penta | [74, 251, 509, 332, 115]
https://gedas.pythonanywhere.com/test/993/
```

10+0.1:
```py
Score of 4k.c-4.28 vs 4ku-2.0: 460 - 265 - 275  [0.598] 1000
...      4k.c-4.28 playing White: 286 - 88 - 126  [0.698] 500
...      4k.c-4.28 playing Black: 174 - 177 - 149  [0.497] 500
...      White vs Black: 463 - 262 - 275  [0.601] 1000
Elo difference: 68.6 +/- 18.5, LOS: 100.0 %, DrawRatio: 27.5 %
```

10+0.1:
```py
Score of 4k.c-4.28 vs 4ku-3.0: 254 - 419 - 327  [0.417] 1000
...      4k.c-4.28 playing White: 179 - 140 - 181  [0.539] 500
...      4k.c-4.28 playing Black: 75 - 279 - 146  [0.296] 500
...      White vs Black: 458 - 215 - 327  [0.622] 1000
Elo difference: -57.9 +/- 17.8, LOS: 0.0 %, DrawRatio: 32.7 %
```

60+0.6:
```py
Score of 4k.c-4.28 vs 4ku-3.0: 1629 - 2136 - 2235  [0.458] 6000
...      4k.c-4.28 playing White: 1194 - 677 - 1129  [0.586] 3000
...      4k.c-4.28 playing Black: 435 - 1459 - 1106  [0.329] 3000
...      White vs Black: 2653 - 1112 - 2235  [0.628] 6000
Elo difference: -29.4 +/- 7.0, LOS: 0.0 %, DrawRatio: 37.3 %
```

### 4.29

64 bit: 4047 bytes (-7)

MD5: 4990d7cdc921c9e4333aaca6abe3338d

Adjust max material

```py
info depth 1 score cp 25 time 0 nodes 20 pv b1c3
info depth 2 score cp 16 time 0 nodes 103 pv b1c3
info depth 3 score cp 23 time 0 nodes 214 pv b1c3
info depth 4 score cp 16 time 0 nodes 419 pv b1c3
info depth 5 score cp 4 time 1 nodes 1044 nps 1044000 pv b1c3
info depth 6 score cp 16 time 2 nodes 1807 nps 903500 pv b1c3
info depth 7 score cp 13 time 4 nodes 4078 nps 1019500 pv b1c3
info depth 8 score cp 16 time 9 nodes 9237 nps 1026333 pv b1c3
info depth 9 score cp 18 time 17 nodes 18631 nps 1095941 pv b1c3
info depth 10 score cp 21 time 28 nodes 34628 nps 1236714 pv b1c3
info depth 11 score cp 3 time 51 nodes 70615 nps 1384607 pv b1c3
info depth 12 score cp 16 time 82 nodes 135256 nps 1649463 pv b1c3
info depth 13 score cp 9 time 196 nodes 391622 nps 1998071 pv b1c3
info depth 14 score cp 16 time 466 nodes 1018157 nps 2184886 pv d2d4
info depth 15 score cp 12 time 1298 nodes 2922164 nps 2251281 pv b1c3
info depth 16 score cp 16 time 2012 nodes 4621241 nps 2296839 pv g1f3
info depth 17 score cp 16 time 2764 nodes 6389421 nps 2311657 pv g1f3
info depth 18 score cp 14 time 4259 nodes 9907003 nps 2326133 pv g1f3
info depth 19 score cp 14 time 6234 nodes 14593619 nps 2340971 pv g1f3
bestmove g1f3
14593619 nodes 2340971 nps
```

```py
Elo   | 3.39 +- 4.92 (95%)
SPRT  | 10.0+0.10s Threads=1 Hash=1MB
LLR   | 2.95 (-2.94, 2.94) [-5.00, 0.00]
Games | N: 9828 W: 3005 L: 2909 D: 3914
Penta | [330, 1122, 1939, 1168, 355]
https://gedas.pythonanywhere.com/test/995/
```

```py
Elo   | 5.76 +- 12.72 (95%)
SPRT  | 60.0+0.60s Threads=1 Hash=1MB
LLR   | 0.62 (-2.94, 2.94) [-5.00, 0.00]
Games | N: 1388 W: 420 L: 397 D: 571
Penta | [34, 179, 253, 186, 42]
https://gedas.pythonanywhere.com/test/997/
```

### 4.30

64 bit: 4078 bytes (+31)

MD5: 8c3b3d58f20be7355096bd25897f1ea0

Mobility without opponent pawn attack

```py
info depth 1 score cp 23 time 0 nodes 20 pv b1c3
info depth 2 score cp 16 time 0 nodes 103 pv b1c3
info depth 3 score cp 23 time 0 nodes 226 pv b1c3
info depth 4 score cp 16 time 0 nodes 493 pv b1c3
info depth 5 score cp 8 time 1 nodes 1290 nps 1290000 pv b1c3
info depth 6 score cp 16 time 2 nodes 2513 nps 1256500 pv b1c3
info depth 7 score cp 14 time 4 nodes 4485 nps 1121250 pv b1c3
info depth 8 score cp 16 time 10 nodes 9646 nps 964600 pv b1c3
info depth 9 score cp 20 time 21 nodes 23404 nps 1114476 pv b1c3
info depth 10 score cp 23 time 37 nodes 47406 nps 1281243 pv b1c3
info depth 11 score cp 7 time 73 nodes 117269 nps 1606424 pv b1c3
info depth 12 score cp 19 time 103 nodes 180183 nps 1749349 pv b1c3
info depth 13 score cp 6 time 436 nodes 957514 nps 2196133 pv c2c4
info depth 14 score cp 22 time 685 nodes 1536627 nps 2243251 pv g1f3
info depth 15 score cp 7 time 1261 nodes 2920142 nps 2315735 pv b1c3
info depth 16 score cp 15 time 2354 nodes 5566147 nps 2364548 pv e2e4
info depth 17 score cp 24 time 3265 nodes 7728628 nps 2367114 pv e2e4
info depth 18 score cp 24 time 4235 nodes 10044715 nps 2371833 pv e2e4
info depth 19 score cp 28 time 7899 nodes 19003972 nps 2405870 pv e2e4
bestmove e2e4
19003972 nodes 2405870 nps
```

```py
Elo   | 13.39 +- 7.44 (95%)
SPRT  | 10.0+0.10s Threads=1 Hash=1MB
LLR   | 2.98 (-2.94, 2.94) [0.00, 5.00]
Games | N: 4646 W: 1540 L: 1361 D: 1745
Penta | [167, 487, 879, 580, 210]
https://gedas.pythonanywhere.com/test/1012/
```

### 4.31

64 bit: 4063 bytes (-15)

MD5: cf54916ad030e4a3e7debcab32909c30

Reordering to reduce size

### 4.32

64 bit: 4061 bytes (-2)

MD5: 0000bacef71548ed12cf5a5a876e9631

More ordering groups

### 4.33

64 bit: 4048 bytes (-13)

MD5: d157443895a44ab47d119a10c04ade38

4 hours reordering

### 4.34

64 bit: 4093 bytes (+45)

MD5: ab2fd8ea8e224e91df2446b12841794b

Blocked passed pawns evaluation

```py
info depth 1 score cp 23 time 0 nodes 20 pv b1c3
info depth 2 score cp 16 time 0 nodes 103 pv b1c3
info depth 3 score cp 23 time 0 nodes 226 pv b1c3
info depth 4 score cp 16 time 0 nodes 493 pv b1c3
info depth 5 score cp 8 time 1 nodes 1290 nps 1290000 pv b1c3
info depth 6 score cp 17 time 2 nodes 2080 nps 1040000 pv b1c3
info depth 7 score cp 10 time 4 nodes 4964 nps 1241000 pv b1c3
info depth 8 score cp 17 time 11 nodes 11218 nps 1019818 pv b1c3
info depth 9 score cp 5 time 33 nodes 41679 nps 1263000 pv d2d4
info depth 10 score cp 28 time 46 nodes 65296 nps 1419478 pv d2d4
info depth 11 score cp 12 time 73 nodes 119948 nps 1643123 pv d2d4
info depth 12 score cp 19 time 101 nodes 182899 nps 1810881 pv d2d4
info depth 13 score cp 16 time 198 nodes 416936 nps 2105737 pv d2d4
info depth 14 score cp 15 time 349 nodes 772940 nps 2214727 pv d2d4
info depth 15 score cp 18 time 1246 nodes 2967140 nps 2381332 pv c2c4
info depth 16 score cp 18 time 1622 nodes 3847734 nps 2372215 pv c2c4
info depth 17 score cp 12 time 3200 nodes 7635156 nps 2385986 pv b1c3
info depth 18 score cp 19 time 6579 nodes 15910871 nps 2418433 pv e2e4
info depth 19 score cp 28 time 9377 nodes 22955422 nps 2448056 pv e2e4
bestmove e2e4
22955422 nodes 2448056 nps
```

```py
Elo   | 23.06 +- 9.89 (95%)
SPRT  | 10.0+0.10s Threads=1 Hash=1MB
LLR   | 3.03 (-2.94, 2.94) [0.00, 5.00]
Games | N: 2520 W: 871 L: 704 D: 945
Penta | [72, 255, 483, 334, 116]
https://gedas.pythonanywhere.com/test/1018/
```

```py
Elo   | 18.60 +- 8.82 (95%)
SPRT  | 60.0+0.60s Threads=1 Hash=1MB
LLR   | 2.95 (-2.94, 2.94) [0.00, 5.00]
Games | N: 3010 W: 944 L: 783 D: 1283
Penta | [80, 319, 588, 396, 122]
https://gedas.pythonanywhere.com/test/1019/
```

### 4.35

64 bit: 4085 bytes (-8)

MD5: d5eca42c25e60d64ca023ad8da443925

Reduce size by reordering


### 4.36

64 bit: 4084 bytes (-1)

MD5: 0310504f3fe9d9831395c39145554342

Knight mobility

```py
info depth 1 score cp 31 time 0 nodes 20 pv b1c3
info depth 2 score cp 16 time 0 nodes 104 pv b1c3
info depth 3 score cp 31 time 0 nodes 220 pv b1c3
info depth 4 score cp 16 time 0 nodes 396 pv b1c3
info depth 5 score cp 15 time 1 nodes 808 nps 808000 pv b1c3
info depth 6 score cp 12 time 2 nodes 2009 nps 1004500 pv b1c3
info depth 7 score cp 14 time 7 nodes 7612 nps 1087428 pv d2d4
info depth 8 score cp 20 time 13 nodes 14277 nps 1098230 pv d2d4
info depth 9 score cp 9 time 27 nodes 31736 nps 1175407 pv d2d4
info depth 10 score cp 26 time 56 nodes 83108 nps 1484071 pv b1c3
info depth 11 score cp 20 time 92 nodes 164073 nps 1783402 pv d2d4
info depth 12 score cp 22 time 145 nodes 289796 nps 1998593 pv d2d4
info depth 13 score cp 9 time 299 nodes 659711 nps 2206391 pv d2d4
info depth 14 score cp 19 time 543 nodes 1243836 nps 2290674 pv e2e4
info depth 15 score cp 19 time 811 nodes 1884270 nps 2323390 pv e2e4
info depth 16 score cp 19 time 1172 nodes 2732615 nps 2331582 pv e2e4
info depth 17 score cp 19 time 1760 nodes 4137942 nps 2351103 pv e2e4
info depth 18 score cp 29 time 3217 nodes 7735204 nps 2404477 pv e2e4
info depth 19 score cp 27 time 4413 nodes 10685597 nps 2421390 pv e2e4
bestmove e2e4
10685597 nodes 2421390 nps
```

```py
Elo   | 9.09 +- 5.81 (95%)
SPRT  | 10.0+0.10s Threads=1 Hash=1MB
LLR   | 2.96 (-2.94, 2.94) [0.00, 5.00]
Games | N: 6918 W: 2189 L: 2008 D: 2721
Penta | [199, 792, 1357, 851, 260]
https://gedas.pythonanywhere.com/test/1046/
```

10+0.1:
```py
Score of 4k.c-4.36 vs 4ku-3.0: 537 - 819 - 644  [0.429] 2000
...      4k.c-4.36 playing White: 377 - 292 - 331  [0.542] 1000
...      4k.c-4.36 playing Black: 160 - 527 - 313  [0.317] 1000
...      White vs Black: 904 - 452 - 644  [0.613] 2000
Elo difference: -49.3 +/- 12.6, LOS: 0.0 %, DrawRatio: 32.2 %
```

60+0.6:
```py
Score of 4k.c-4.36 vs 4ku-3.0: 3662 - 3925 - 4413  [0.489] 12000
...      4k.c-4.36 playing White: 2641 - 1128 - 2232  [0.626] 6001
...      4k.c-4.36 playing Black: 1021 - 2797 - 2181  [0.352] 5999
...      White vs Black: 5438 - 2149 - 4413  [0.637] 12000
Elo difference: -7.6 +/- 4.9, LOS: 0.1 %, DrawRatio: 36.8 %
```

### 4.37

64 bit: 4069 bytes (-15)

MD5: 87a87f858fe641e1ebe026452772dd5a

Static toggling

### 4.38

64 bit: 4068 bytes (-1)

MD5: c138f6e71d8404b6b7f0a9d2b35eeddf

More passers

```py
info depth 1 score cp 30 time 0 nodes 20 pv b1c3
info depth 2 score cp 16 time 0 nodes 104 pv b1c3
info depth 3 score cp 30 time 0 nodes 240 pv b1c3
info depth 4 score cp 17 time 0 nodes 417 pv b1c3
info depth 5 score cp 14 time 1 nodes 1282 nps 1282000 pv d2d4
info depth 6 score cp 13 time 2 nodes 2122 nps 1061000 pv d2d4
info depth 7 score cp 13 time 4 nodes 4538 nps 1134500 pv d2d4
info depth 8 score cp 16 time 13 nodes 15254 nps 1173384 pv d2d4
info depth 9 score cp 12 time 28 nodes 37174 nps 1327642 pv d2d4
info depth 10 score cp 28 time 43 nodes 63411 nps 1474674 pv d2d4
info depth 11 score cp 15 time 68 nodes 111340 nps 1637352 pv d2d4
info depth 12 score cp 13 time 117 nodes 221492 nps 1893094 pv d2d4
info depth 13 score cp 11 time 210 nodes 444028 nps 2114419 pv d2d4
info depth 14 score cp 16 time 356 nodes 798352 nps 2242561 pv d2d4
info depth 15 score cp 33 time 1068 nodes 2545838 nps 2383743 pv e2e4
info depth 16 score cp 36 time 1597 nodes 3854557 nps 2413623 pv e2e4
info depth 17 score cp 29 time 2363 nodes 5713620 nps 2417951 pv e2e4
info depth 18 score cp 33 time 3239 nodes 7918294 nps 2444672 pv e2e4
info depth 19 score cp 33 time 4750 nodes 11777690 nps 2479513 pv e2e4
bestmove e2e4
11777690 nodes 2479513 nps
```

```py
Elo   | 1.94 +- 1.80 (95%)
SPRT  | 10.0+0.10s Threads=1 Hash=1MB
LLR   | 0.71 (-2.94, 2.94) [0.00, 5.00]
Games | N: 72170 W: 21884 L: 21480 D: 28806
Penta | [2355, 8478, 14146, 8620, 2486]
https://gedas.pythonanywhere.com/test/1069/
```

### 4.39

64 bit: 4096 bytes (+28)

MD5: 26394c94e07545923a4ca654bdbed7b6

Attacked by pawns penalty

```py
info depth 1 score cp 30 time 0 nodes 20 pv b1c3
info depth 2 score cp 16 time 0 nodes 104 pv b1c3
info depth 3 score cp 30 time 0 nodes 240 pv b1c3
info depth 4 score cp 17 time 0 nodes 417 pv b1c3
info depth 5 score cp 14 time 1 nodes 1275 nps 1275000 pv d2d4
info depth 6 score cp 13 time 2 nodes 2090 nps 1045000 pv d2d4
info depth 7 score cp 13 time 5 nodes 4529 nps 905800 pv d2d4
info depth 8 score cp 16 time 12 nodes 10492 nps 874333 pv d2d4
info depth 9 score cp 13 time 28 nodes 29688 nps 1060285 pv d2d4
info depth 10 score cp 23 time 42 nodes 51218 nps 1219476 pv d2d4
info depth 11 score cp 20 time 65 nodes 91353 nps 1405430 pv d2d4
info depth 12 score cp 21 time 116 nodes 201833 nps 1739939 pv d2d4
info depth 13 score cp 11 time 231 nodes 477920 nps 2068917 pv d2d4
info depth 14 score cp 24 time 475 nodes 1043961 nps 2197812 pv e2e4
info depth 15 score cp 24 time 636 nodes 1423048 nps 2237496 pv e2e4
info depth 16 score cp 34 time 1263 nodes 3024248 nps 2394495 pv e2e4
info depth 17 score cp 35 time 2112 nodes 5117451 nps 2423035 pv e2e4
info depth 18 score cp 38 time 3267 nodes 7992566 nps 2446454 pv e2e4
info depth 19 score cp 32 time 4562 nodes 11249123 nps 2465831 pv e2e4
bestmove e2e4
11249123 nodes 2465831 nps
```

```py
Elo   | 14.95 +- 7.83 (95%)
SPRT  | 10.0+0.10s Threads=1 Hash=1MB
LLR   | 2.95 (-2.94, 2.94) [0.00, 5.00]
Games | N: 3884 W: 1254 L: 1087 D: 1543
Penta | [113, 432, 715, 539, 143]
https://gedas.pythonanywhere.com/test/1072/
```

60+0.6:
```py
Score of 4k.c-4.39 vs 4ku-3.0: 2663 - 2529 - 3308  [0.508] 8500
...      4k.c-4.39 playing White: 1928 - 742 - 1581  [0.639] 4251
...      4k.c-4.39 playing Black: 735 - 1787 - 1727  [0.376] 4249
...      White vs Black: 3715 - 1477 - 3308  [0.632] 8500
Elo difference: 5.5 +/- 5.8, LOS: 96.9 %, DrawRatio: 38.9 %
```

### 4.40

64 bit: 4092 bytes (-4)

MD5: dfc2f39e9da8e631814704fb07d00180

Smaller best move finding

### 4.41

64 bit: 4094 bytes (+2)

MD5: 2a0b981bd48fd77406cdf4a86b2b8112

Tune parameters

```py
info depth 1 score cp 30 time 0 nodes 20 pv b1c3
info depth 2 score cp 16 time 1 nodes 104 nps 104000 pv b1c3
info depth 3 score cp 30 time 1 nodes 241 nps 241000 pv b1c3
info depth 4 score cp 17 time 1 nodes 419 nps 419000 pv b1c3
info depth 5 score cp 14 time 3 nodes 1279 nps 426333 pv d2d4
info depth 6 score cp 13 time 4 nodes 2125 nps 531250 pv d2d4
info depth 7 score cp 13 time 9 nodes 4495 nps 499444 pv d2d4
info depth 8 score cp 16 time 17 nodes 10341 nps 608294 pv d2d4
info depth 9 score cp 13 time 34 nodes 22835 nps 671617 pv d2d4
info depth 10 score cp 4 time 64 nodes 51754 nps 808656 pv d2d4
info depth 11 score cp 10 time 137 nodes 141739 nps 1034591 pv c2c4
info depth 12 score cp 22 time 230 nodes 270855 nps 1177630 pv c2c4
info depth 13 score cp 11 time 451 nodes 593657 nps 1316312 pv c2c4
info depth 14 score cp 16 time 767 nodes 1068012 nps 1392453 pv g1f3
info depth 15 score cp 16 time 1664 nodes 2427556 nps 1458867 pv e2e4
info depth 16 score cp 30 time 2282 nodes 3361541 nps 1473067 pv e2e4
info depth 17 score cp 28 time 3277 nodes 4890281 nps 1492304 pv e2e4
info depth 18 score cp 28 time 4457 nodes 6704399 nps 1504240 pv e2e4
info depth 19 score cp 26 time 7301 nodes 11165926 nps 1529369 pv e2e4
bestmove e2e4
11165926 nodes 1529369 nps
```

```py
Elo   | 2.62 +- 1.94 (95%)
SPRT  | 10.0+0.10s Threads=1 Hash=1MB
LLR   | 3.04 (-2.94, 2.94) [0.00, 5.00]
Games | N: 61872 W: 18584 L: 18118 D: 25170
Penta | [1988, 7259, 12133, 7411, 2145]
https://gedas.pythonanywhere.com/test/1094/
```

```py
Elo   | 2.32 +- 1.65 (95%)
SPRT  | 60.0+0.60s Threads=1 Hash=1MB
LLR   | 3.00 (-2.94, 2.94) [0.00, 5.00]
Games | N: 78530 W: 22890 L: 22365 D: 33275
Penta | [2115, 9217, 16216, 9462, 2255]
https://gedas.pythonanywhere.com/test/1095/
```

### 4.42

64 bit: 4087 bytes (-7)

MD5: 7fd1434018c9c65fcc1d674ccff8e3c8

Include castling permissions into hash

```py
info depth 1 score cp 30 time 0 nodes 20 pv b1c3
info depth 2 score cp 16 time 0 nodes 104 pv b1c3
info depth 3 score cp 30 time 1 nodes 241 nps 241000 pv b1c3
info depth 4 score cp 17 time 1 nodes 419 nps 419000 pv b1c3
info depth 5 score cp 14 time 2 nodes 1279 nps 639500 pv d2d4
info depth 6 score cp 13 time 2 nodes 2138 nps 1069000 pv d2d4
info depth 7 score cp 13 time 5 nodes 4559 nps 911800 pv d2d4
info depth 8 score cp 16 time 11 nodes 10805 nps 982272 pv d2d4
info depth 9 score cp 25 time 23 nodes 27461 nps 1193956 pv d2d4
info depth 10 score cp 16 time 39 nodes 54271 nps 1391564 pv d2d4
info depth 11 score cp 14 time 56 nodes 88237 nps 1575660 pv d2d4
info depth 12 score cp 16 time 89 nodes 163317 nps 1835022 pv d2d4
info depth 13 score cp 10 time 213 nodes 472068 nps 2216281 pv d2d4
info depth 14 score cp 16 time 317 nodes 728251 nps 2297321 pv d2d4
info depth 15 score cp 22 time 692 nodes 1703250 nps 2461343 pv e2e4
info depth 16 score cp 31 time 1085 nodes 2728202 nps 2514471 pv e2e4
info depth 17 score cp 31 time 1472 nodes 3677791 nps 2498499 pv e2e4
info depth 18 score cp 34 time 2772 nodes 6977987 nps 2517311 pv e2e4
info depth 19 score cp 30 time 4357 nodes 10897277 nps 2501096 pv e2e4
bestmove e2e4
10897277 nodes 2501096 nps
```

```py
Elo   | -0.07 +- 2.75 (95%)
SPRT  | 10.0+0.10s Threads=1 Hash=1MB
LLR   | 3.08 (-2.94, 2.94) [-5.00, 0.00]
Games | N: 30882 W: 9149 L: 9155 D: 12578
Penta | [1039, 3658, 6019, 3720, 1005]
https://gedas.pythonanywhere.com/test/1120/
```

```py
Elo   | -0.29 +- 2.47 (95%)
SPRT  | 60.0+0.60s Threads=1 Hash=1MB
LLR   | 2.95 (-2.94, 2.94) [-5.00, 0.00]
Games | N: 34468 W: 9953 L: 9982 D: 14533
Penta | [899, 4173, 7174, 4034, 954]
https://gedas.pythonanywhere.com/test/1121/
```

### 4.43

64 bit: 4084 bytes (-3)

MD5: 420557e930addd6476915d4235f5ef24

Smaller decompressor

### 4.44

64 bit: 4083 bytes (-1)

MD5: 0784ca2fae6987cca522fd3a2e842182

Tune parameters

```py
info depth 1 score cp 29 time 0 nodes 20 pv b1c3
info depth 2 score cp 17 time 0 nodes 104 pv b1c3
info depth 3 score cp 29 time 0 nodes 241 pv b1c3
info depth 4 score cp 18 time 0 nodes 419 pv b1c3
info depth 5 score cp 13 time 1 nodes 1294 nps 1294000 pv d2d4
info depth 6 score cp 14 time 2 nodes 2212 nps 1106000 pv d2d4
info depth 7 score cp 12 time 4 nodes 4569 nps 1142250 pv d2d4
info depth 8 score cp 17 time 10 nodes 10321 nps 1032100 pv d2d4
info depth 9 score cp 12 time 30 nodes 38065 nps 1268833 pv d2d4
info depth 10 score cp 29 time 46 nodes 66058 nps 1436043 pv d2d4
info depth 11 score cp 14 time 104 nodes 196217 nps 1886701 pv g1f3
info depth 12 score cp 16 time 170 nodes 344900 nps 2028823 pv g1f3
info depth 13 score cp 16 time 281 nodes 600342 nps 2136448 pv g1f3
info depth 14 score cp 13 time 524 nodes 1177076 nps 2246328 pv g1f3
info depth 15 score cp 10 time 934 nodes 2165457 nps 2318476 pv g1f3
info depth 16 score cp 28 time 2118 nodes 5090576 nps 2403482 pv e2e4
info depth 17 score cp 30 time 3104 nodes 7611209 nps 2452064 pv e2e4
info depth 18 score cp 20 time 4772 nodes 11897633 nps 2493217 pv e2e4
info depth 19 score cp 32 time 6633 nodes 16837823 nps 2538492 pv e2e4
bestmove e2e4
16837823 nodes 2538492 nps
```

```py
Elo   | 2.54 +- 1.88 (95%)
SPRT  | 10.0+0.10s Threads=1 Hash=1MB
LLR   | 2.95 (-2.94, 2.94) [0.00, 5.00]
Games | N: 66678 W: 19983 L: 19496 D: 27199
Penta | [2228, 7675, 13090, 8074, 2272]
https://gedas.pythonanywhere.com/test/1135/
```

```py
Elo   | 2.67 +- 2.07 (95%)
SPRT  | 60.0+0.60s Threads=1 Hash=1MB
LLR   | 2.97 (-2.94, 2.94) [0.00, 5.00]
Games | N: 49232 W: 14331 L: 13953 D: 20948
Penta | [1301, 5765, 10126, 6103, 1321]
https://gedas.pythonanywhere.com/test/1136/
```

### 4.46

64 bit: 4094 bytes (+11)

MD5: 8fe28a6e333db96423318817d472d38a

Faster move ordering

```py
info depth 1 score cp 29 time 1 nodes 20 nps 20000 pv b1c3
info depth 2 score cp 17 time 1 nodes 104 nps 104000 pv b1c3
info depth 3 score cp 29 time 1 nodes 241 nps 241000 pv b1c3
info depth 4 score cp 18 time 1 nodes 423 nps 423000 pv b1c3
info depth 5 score cp 13 time 2 nodes 1391 nps 695500 pv d2d4
info depth 6 score cp 14 time 3 nodes 2677 nps 892333 pv d2d4
info depth 7 score cp 12 time 5 nodes 4810 nps 962000 pv d2d4
info depth 8 score cp 17 time 13 nodes 13461 nps 1035461 pv d2d4
info depth 9 score cp 12 time 25 nodes 28177 nps 1127080 pv d2d4
info depth 10 score cp 17 time 48 nodes 69376 nps 1445333 pv g1f3
info depth 11 score cp 18 time 69 nodes 111898 nps 1621710 pv g1f3
info depth 12 score cp 17 time 114 nodes 222911 nps 1955359 pv g1f3
info depth 13 score cp 10 time 395 nodes 930113 nps 2354716 pv g1f3
info depth 14 score cp 13 time 572 nodes 1361241 nps 2379791 pv g1f3
info depth 15 score cp 23 time 1072 nodes 2651401 nps 2473321 pv e2e4
info depth 16 score cp 23 time 1332 nodes 3329644 nps 2499732 pv e2e4
info depth 17 score cp 23 time 1751 nodes 4433260 nps 2531844 pv e2e4
info depth 18 score cp 28 time 2732 nodes 7041462 nps 2577401 pv e2e4
info depth 19 score cp 28 time 4136 nodes 10735613 nps 2595651 pv e2e4
bestmove e2e4
10735613 nodes 2595651 nps
```

```py
Elo   | 9.28 +- 5.88 (95%)
SPRT  | 10.0+0.10s Threads=1 Hash=1MB
LLR   | 2.95 (-2.94, 2.94) [0.00, 5.00]
Games | N: 6700 W: 2079 L: 1900 D: 2721
Penta | [199, 740, 1343, 819, 249]
https://gedas.pythonanywhere.com/test/1149/
```

```py
Elo   | 9.19 +- 5.68 (95%)
SPRT  | 60.0+0.60s Threads=1 Hash=1MB
LLR   | 2.97 (-2.94, 2.94) [0.00, 5.00]
Games | N: 6314 W: 1914 L: 1747 D: 2653
Penta | [130, 734, 1313, 799, 181]
https://gedas.pythonanywhere.com/test/1150/
```

Local 10+0.1 test on mini version:
```py
Score of 4kc-4.45-mini vs 4kc-4.44-mini: 35157 - 31588 - 45255  [0.516] 112000
...      4kc-4.45-mini playing White: 24900 - 8886 - 22212  [0.643] 55998
...      4kc-4.45-mini playing Black: 10257 - 22702 - 23043  [0.389] 56002
...      White vs Black: 47602 - 19143 - 45255  [0.627] 112000
Elo difference: 11.1 +/- 1.6, LOS: 100.0 %, DrawRatio: 40.4 %
```

### 4.47

64 bit: 4072 bytes (-22)

MD5: 3f1111f4606d2cebf5aed7dd5af56ac2

Smaller LMR

```py
info depth 1 score cp 29 time 0 nodes 20 pv b1c3
info depth 2 score cp 17 time 0 nodes 104 pv b1c3
info depth 3 score cp 29 time 0 nodes 241 pv b1c3
info depth 4 score cp 18 time 0 nodes 423 pv b1c3
info depth 5 score cp 13 time 1 nodes 1391 nps 1391000 pv d2d4
info depth 6 score cp 14 time 3 nodes 2677 nps 892333 pv d2d4
info depth 7 score cp 12 time 4 nodes 4810 nps 1202500 pv d2d4
info depth 8 score cp 17 time 12 nodes 13461 nps 1121750 pv d2d4
info depth 9 score cp 12 time 24 nodes 28177 nps 1174041 pv d2d4
info depth 10 score cp 17 time 48 nodes 69376 nps 1445333 pv g1f3
info depth 11 score cp 18 time 67 nodes 111898 nps 1670119 pv g1f3
info depth 12 score cp 17 time 111 nodes 222911 nps 2008207 pv g1f3
info depth 13 score cp 10 time 378 nodes 930113 nps 2460616 pv g1f3
info depth 14 score cp 13 time 547 nodes 1361241 nps 2488557 pv g1f3
info depth 15 score cp 23 time 1035 nodes 2651401 nps 2561740 pv e2e4
info depth 16 score cp 23 time 1288 nodes 3329644 nps 2585127 pv e2e4
info depth 17 score cp 23 time 1698 nodes 4433260 nps 2610871 pv e2e4
info depth 18 score cp 28 time 2655 nodes 7041462 nps 2652151 pv e2e4
info depth 19 score cp 28 time 4011 nodes 10735613 nps 2676542 pv e2e4
bestmove e2e4
10735613 nodes 2676542 nps
```

```py
Elo   | -0.05 +- 3.97 (95%)
SPRT  | 10.0+0.10s Threads=1 Hash=1MB
LLR   | 1.43 (-2.94, 2.94) [-5.00, 0.00]
Games | N: 14248 W: 4214 L: 4216 D: 5818
Penta | [444, 1683, 2859, 1707, 431]
https://gedas.pythonanywhere.com/test/1156/
```

Local test for 10+0.01 mini:
```py
Score of 4kc-4.47-mini vs 4kc-4.46-mini: 1835 - 1802 - 2363  [0.503] 6000
...      4kc-4.47-mini playing White: 1279 - 546 - 1176  [0.622] 3001
...      4kc-4.47-mini playing Black: 556 - 1256 - 1187  [0.383] 2999
...      White vs Black: 2535 - 1102 - 2363  [0.619] 6000
Elo difference: 1.9 +/- 6.8, LOS: 70.8 %, DrawRatio: 39.4 %
```

### 4.48

64 bit: 4068 bytes (-4)

MD5: e5c687e5e0f6e6f97d5ea821e120fda6

No history bonus in qsearch

-Oz:
```py
info depth 6 nodes 119060324 time 5684 nps 20946573
```

-O3:
```py
perft 6
info depth 6 nodes 119060324 time 2984 nps 39899572
```

```py
info depth 1 score cp 29 time 0 nodes 20 pv b1c3
info depth 2 score cp 17 time 0 nodes 104 pv b1c3
info depth 3 score cp 29 time 0 nodes 241 pv b1c3
info depth 4 score cp 18 time 1 nodes 423 nps 423000 pv b1c3
info depth 5 score cp 13 time 2 nodes 1391 nps 695500 pv d2d4
info depth 6 score cp 14 time 3 nodes 2673 nps 891000 pv d2d4
info depth 7 score cp 12 time 5 nodes 4803 nps 960600 pv d2d4
info depth 8 score cp 17 time 14 nodes 13932 nps 995142 pv d2d4
info depth 9 score cp 12 time 23 nodes 24208 nps 1052521 pv d2d4
info depth 10 score cp 17 time 35 nodes 41218 nps 1177657 pv d2d4
info depth 11 score cp 28 time 61 nodes 94294 nps 1545803 pv d2d4
info depth 12 score cp 32 time 108 nodes 200535 nps 1856805 pv d2d4
info depth 13 score cp 20 time 179 nodes 369525 nps 2064385 pv d2d4
info depth 14 score cp 17 time 293 nodes 639838 nps 2183747 pv d2d4
info depth 15 score cp 33 time 727 nodes 1698375 nps 2336141 pv e2e4
info depth 16 score cp 24 time 1030 nodes 2446315 nps 2375063 pv e2e4
info depth 17 score cp 26 time 1578 nodes 3851763 nps 2440914 pv e2e4
info depth 18 score cp 26 time 2440 nodes 6097287 nps 2498888 pv e2e4
info depth 19 score cp 26 time 3951 nodes 9966603 nps 2522552 pv e2e4
bestmove e2e4
9966603 nodes 2522552 nps
```

```py
Elo   | -1.93 +- 4.08 (95%)
SPRT  | 10.0+0.10s Threads=1 Hash=1MB
LLR   | -2.97 (-2.94, 2.94) [0.00, 5.00]
Games | N: 13652 W: 4041 L: 4117 D: 5494
Penta | [450, 1623, 2722, 1615, 416]
https://gedas.pythonanywhere.com/test/1174/
```

```py
Elo   | 0.41 +- 3.03 (95%)
SPRT  | 60.0+0.60s Threads=1 Hash=1MB
LLR   | 2.95 (-2.94, 2.94) [-5.00, 0.00]
Games | N: 22930 W: 6534 L: 6507 D: 9889
Penta | [601, 2771, 4716, 2754, 623]
https://gedas.pythonanywhere.com/test/1178/
```

### 4.49

64 bit: 4061 bytes (-7)

MD5: 914cd3d79b0b9437c42bac11c63ebac0

Simplify is_attacked and speedup

-Oz:
```py
perft 6
info depth 6 nodes 119060324 time 7317 nps 16271740
```

-O3:
```py
perft 6
info depth 6 nodes 119060324 time 3799 nps 31339911
```

```py
info depth 1 score cp 29 time 0 nodes 20 pv b1c3
info depth 2 score cp 17 time 0 nodes 104 pv b1c3
info depth 3 score cp 29 time 0 nodes 241 pv b1c3
info depth 4 score cp 18 time 1 nodes 423 nps 423000 pv b1c3
info depth 5 score cp 13 time 2 nodes 1391 nps 695500 pv d2d4
info depth 6 score cp 14 time 3 nodes 2673 nps 891000 pv d2d4
info depth 7 score cp 12 time 5 nodes 4803 nps 960600 pv d2d4
info depth 8 score cp 17 time 14 nodes 13932 nps 995142 pv d2d4
info depth 9 score cp 12 time 23 nodes 24208 nps 1052521 pv d2d4
info depth 10 score cp 17 time 35 nodes 41218 nps 1177657 pv d2d4
info depth 11 score cp 28 time 61 nodes 94294 nps 1545803 pv d2d4
info depth 12 score cp 32 time 108 nodes 200535 nps 1856805 pv d2d4
info depth 13 score cp 20 time 179 nodes 369525 nps 2064385 pv d2d4
info depth 14 score cp 17 time 293 nodes 639838 nps 2183747 pv d2d4
info depth 15 score cp 33 time 727 nodes 1698375 nps 2336141 pv e2e4
info depth 16 score cp 24 time 1030 nodes 2446315 nps 2375063 pv e2e4
info depth 17 score cp 26 time 1578 nodes 3851763 nps 2440914 pv e2e4
info depth 18 score cp 26 time 2440 nodes 6097287 nps 2498888 pv e2e4
info depth 19 score cp 26 time 3951 nodes 9966603 nps 2522552 pv e2e4
bestmove e2e4
9966603 nodes 2522552 nps
```

```py
Elo   | 0.90 +- 6.25 (95%)
SPRT  | 10.0+0.10s Threads=1 Hash=1MB
LLR   | 0.91 (-2.94, 2.94) [-5.00, 0.00]
Games | N: 5776 W: 1740 L: 1725 D: 2311
Penta | [171, 678, 1208, 627, 204]
https://gedas.pythonanywhere.com/test/1182/
```

### 4.50

64 bit: 4058 bytes (-3)

MD5: a21efc7fbfe1660bb6aae320486c6704

Exclude pawn control for king attacks

```py
info depth 1 score cp 30 time 0 nodes 20 pv b1c3
info depth 2 score cp 17 time 0 nodes 103 pv b1c3
info depth 3 score cp 30 time 1 nodes 219 nps 219000 pv b1c3
info depth 4 score cp 17 time 1 nodes 402 nps 402000 pv b1c3
info depth 5 score cp 14 time 1 nodes 909 nps 909000 pv b1c3
info depth 6 score cp 13 time 2 nodes 2100 nps 1050000 pv b1c3
info depth 7 score cp 9 time 7 nodes 6737 nps 962428 pv g1f3
info depth 8 score cp 17 time 18 nodes 21178 nps 1176555 pv b1c3
info depth 9 score cp 11 time 33 nodes 42894 nps 1299818 pv b1c3
info depth 10 score cp 17 time 45 nodes 65319 nps 1451533 pv b1c3
info depth 11 score cp 14 time 70 nodes 120859 nps 1726557 pv b1c3
info depth 12 score cp 13 time 118 nodes 238400 nps 2020338 pv b1c3
info depth 13 score cp 9 time 308 nodes 742883 nps 2411957 pv b1c3
info depth 14 score cp 29 time 644 nodes 1651674 nps 2564711 pv e2e4
info depth 15 score cp 36 time 944 nodes 2469611 nps 2616113 pv e2e4
info depth 16 score cp 27 time 1219 nodes 3206706 nps 2630603 pv e2e4
info depth 17 score cp 27 time 1747 nodes 4628135 nps 2649190 pv e2e4
info depth 18 score cp 30 time 2433 nodes 6496429 nps 2670131 pv e2e4
info depth 19 score cp 19 time 5567 nodes 15155261 nps 2722338 pv e2e4
bestmove e2e4
15155261 nodes 2722338 nps
```

```py
Elo   | -0.01 +- 2.87 (95%)
SPRT  | 10.0+0.10s Threads=1 Hash=1MB
LLR   | -2.95 (-2.94, 2.94) [0.00, 5.00]
Games | N: 28256 W: 8381 L: 8382 D: 11493
Penta | [935, 3339, 5577, 3346, 931]
https://gedas.pythonanywhere.com/test/1187/
```

```py
Elo   | 0.78 +- 3.29 (95%)
SPRT  | 60.0+0.60s Threads=1 Hash=1MB
LLR   | 2.97 (-2.94, 2.94) [-5.00, 0.00]
Games | N: 19594 W: 5656 L: 5612 D: 8326
Penta | [531, 2305, 4101, 2309, 551]
https://gedas.pythonanywhere.com/test/1188/
```

### 4.51

64 bit: 4096 bytes (+38)

MD5: ce1fb2aa20ec5b1a0515b0e623f2f58f

Endgame scaling by remaining pawn count

```py
info depth 1 score cp 26 time 0 nodes 20 pv b1c3
info depth 2 score cp 17 time 1 nodes 103 nps 103000 pv b1c3
info depth 3 score cp 26 time 1 nodes 259 nps 259000 pv b1c3
info depth 4 score cp 17 time 1 nodes 484 nps 484000 pv b1c3
info depth 5 score cp 12 time 2 nodes 1261 nps 630500 pv g1f3
info depth 6 score cp 17 time 3 nodes 2698 nps 899333 pv g1f3
info depth 7 score cp 12 time 6 nodes 5152 nps 858666 pv g1f3
info depth 8 score cp 17 time 10 nodes 9968 nps 996800 pv g1f3
info depth 9 score cp 10 time 19 nodes 20987 nps 1104578 pv g1f3
info depth 10 score cp 17 time 32 nodes 39487 nps 1233968 pv g1f3
info depth 11 score cp 18 time 71 nodes 120337 nps 1694887 pv b1c3
info depth 12 score cp 24 time 128 nodes 260181 nps 2032664 pv b1c3
info depth 13 score cp 12 time 231 nodes 520048 nps 2251290 pv b1c3
info depth 14 score cp 30 time 482 nodes 1145781 nps 2377139 pv e2e4
info depth 15 score cp 23 time 683 nodes 1662437 nps 2434021 pv e2e4
info depth 16 score cp 23 time 1054 nodes 2580759 nps 2448537 pv e2e4
info depth 17 score cp 35 time 2184 nodes 5607328 nps 2567457 pv e2e4
info depth 18 score cp 29 time 3125 nodes 8062530 nps 2580009 pv e2e4
info depth 19 score cp 30 time 5682 nodes 14761179 nps 2597884 pv e2e4
bestmove e2e4
14761179 nodes 2597884 nps
```

```py
Elo   | 5.63 +- 4.21 (95%)
SPRT  | 10.0+0.10s Threads=1 Hash=1MB
LLR   | 2.95 (-2.94, 2.94) [0.00, 5.00]
Games | N: 13030 W: 3999 L: 3788 D: 5243
Penta | [415, 1458, 2585, 1615, 442]
https://gedas.pythonanywhere.com/test/1211/
```

```py
Elo   | 18.16 +- 8.50 (95%)
SPRT  | 60.0+0.60s Threads=1 Hash=1MB
LLR   | 2.98 (-2.94, 2.94) [0.00, 5.00]
Games | N: 2988 W: 964 L: 808 D: 1216
Penta | [68, 310, 618, 394, 104]
https://gedas.pythonanywhere.com/test/1212/
```

### 4.52

64 bit: 4096 bytes (=)

MD5: 07f728cb02794ee02366c6a0ad590987

Fix early exit logic

```py
info depth 1 score cp 26 time 0 nodes 20 pv b1c3
info depth 2 score cp 17 time 0 nodes 103 pv b1c3
info depth 3 score cp 26 time 0 nodes 259 pv b1c3
info depth 4 score cp 17 time 0 nodes 483 pv b1c3
info depth 5 score cp 12 time 1 nodes 1260 nps 1260000 pv g1f3
info depth 6 score cp 17 time 3 nodes 2696 nps 898666 pv g1f3
info depth 7 score cp 12 time 6 nodes 5150 nps 858333 pv g1f3
info depth 8 score cp 17 time 10 nodes 9969 nps 996900 pv g1f3
info depth 9 score cp 10 time 20 nodes 20994 nps 1049700 pv g1f3
info depth 10 score cp 17 time 32 nodes 39468 nps 1233375 pv g1f3
info depth 11 score cp 12 time 57 nodes 84521 nps 1482824 pv g1f3
info depth 12 score cp 22 time 140 nodes 279684 nps 1997742 pv d2d4
info depth 13 score cp 15 time 225 nodes 490206 nps 2178693 pv d2d4
info depth 14 score cp 10 time 458 nodes 1061863 nps 2318478 pv d2d4
info depth 15 score cp 24 time 1035 nodes 2531165 nps 2445570 pv e2e4
info depth 16 score cp 29 time 1530 nodes 3792095 nps 2478493 pv e2e4
info depth 17 score cp 29 time 1981 nodes 4984793 nps 2516301 pv e2e4
info depth 18 score cp 28 time 2899 nodes 7445133 nps 2568172 pv e2e4
info depth 19 score cp 30 time 4583 nodes 11905272 nps 2597702 pv e2e4
bestmove e2e4
11905272 nodes 2597702 nps
```

```py
Elo   | 3.05 +- 4.70 (95%)
SPRT  | 10.0+0.10s Threads=1 Hash=1MB
LLR   | 3.00 (-2.94, 2.94) [-5.00, 0.00]
Games | N: 10708 W: 3220 L: 3126 D: 4362
Penta | [346, 1261, 2087, 1273, 387]
https://gedas.pythonanywhere.com/test/1222/
```

```py
Elo   | 13.56 +- 8.14 (95%)
SPRT  | 60.0+0.60s Threads=1 Hash=1MB
LLR   | 2.95 (-2.94, 2.94) [-5.00, 0.00]
Games | N: 3128 W: 908 L: 786 D: 1434
Penta | [64, 346, 658, 396, 100]
https://gedas.pythonanywhere.com/test/1223/
```

### 4.53

64 bit: 4083 bytes (-13)

MD5: 67f4e583e44a21d3fb7582a8c2b56165

Smaller loader

### 4.54

64 bit: 4079 bytes (-4)

MD5: d9db9ee8726aa66e9fb81812c8a2596a

Full ASM loader

### 4.55

64 bit: 4076 bytes (-3)

MD5: 65fe9176c8667e80867e45efb23198af

Remove max_material

### 4.56

64 bit: 4069 bytes (-7)

MD5: e3a442c746421a06cbd11af0af3476f7

Smaller syscalls 

### 4.57

64 bit: 4078 bytes (+9)

MD5: 03da8492562da9095582f124f876c518

More RFP if improving

```py
info depth 1 score cp 26 time 0 nodes 20 pv b1c3
info depth 2 score cp 17 time 0 nodes 103 pv b1c3
info depth 3 score cp 26 time 0 nodes 260 pv b1c3
info depth 4 score cp 17 time 1 nodes 510 nps 510000 pv b1c3
info depth 5 score cp 12 time 1 nodes 1315 nps 1315000 pv d2d4
info depth 6 score cp 17 time 2 nodes 2463 nps 1231500 pv d2d4
info depth 7 score cp 12 time 5 nodes 5791 nps 1158200 pv d2d4
info depth 8 score cp 17 time 10 nodes 11441 nps 1144100 pv d2d4
info depth 9 score cp 10 time 19 nodes 22451 nps 1181631 pv d2d4
info depth 10 score cp 17 time 31 nodes 39715 nps 1281129 pv d2d4
info depth 11 score cp 11 time 67 nodes 103879 nps 1550432 pv g1f3
info depth 12 score cp 9 time 119 nodes 217615 nps 1828697 pv g1f3
info depth 13 score cp 13 time 216 nodes 450859 nps 2087310 pv g1f3
info depth 14 score cp 16 time 376 nodes 842263 nps 2240061 pv g1f3
info depth 15 score cp 24 time 875 nodes 2043366 nps 2335275 pv e2e4
info depth 16 score cp 29 time 1180 nodes 2772398 nps 2349489 pv e2e4
info depth 17 score cp 29 time 1596 nodes 3809306 nps 2386783 pv e2e4
info depth 18 score cp 27 time 2514 nodes 6083950 nps 2420027 pv e2e4
info depth 19 score cp 27 time 3497 nodes 8532027 nps 2439813 pv e2e4
bestmove e2e4
8532027 nodes 2439813 nps
```

```py
Elo   | 6.63 +- 4.68 (95%)
SPRT  | 10.0+0.10s Threads=1 Hash=1MB
LLR   | 3.00 (-2.94, 2.94) [0.00, 5.00]
Games | N: 10372 W: 3125 L: 2927 D: 4320
Penta | [303, 1194, 2031, 1318, 340]
https://gedas.pythonanywhere.com/test/1253/
```

```py
Elo   | 6.17 +- 4.37 (95%)
SPRT  | 60.0+0.60s Threads=1 Hash=1MB
LLR   | 2.95 (-2.94, 2.94) [0.00, 5.00]
Games | N: 10026 W: 2776 L: 2598 D: 4652
Penta | [191, 1177, 2134, 1285, 226]
https://gedas.pythonanywhere.com/test/1255/
```

### 4.58

64 bit: 4095 bytes (+17)

MD5: 4790b0115546b2e9d5e5e1c8e2048c9c

Adjust NMP depth by beta distance to eval

```py
info depth 1 score cp 26 time 0 nodes 20 pv b1c3
info depth 2 score cp 17 time 0 nodes 103 pv b1c3
info depth 3 score cp 26 time 1 nodes 260 nps 260000 pv b1c3
info depth 4 score cp 17 time 1 nodes 510 nps 510000 pv b1c3
info depth 5 score cp 12 time 2 nodes 1315 nps 657500 pv d2d4
info depth 6 score cp 17 time 2 nodes 2463 nps 1231500 pv d2d4
info depth 7 score cp 12 time 6 nodes 5791 nps 965166 pv d2d4
info depth 8 score cp 17 time 10 nodes 11440 nps 1144000 pv d2d4
info depth 9 score cp 10 time 19 nodes 22454 nps 1181789 pv d2d4
info depth 10 score cp 17 time 32 nodes 39707 nps 1240843 pv d2d4
info depth 11 score cp 11 time 72 nodes 108760 nps 1510555 pv g1f3
info depth 12 score cp 20 time 128 nodes 230039 nps 1797179 pv d2d4
info depth 13 score cp 14 time 354 nodes 736547 nps 2080641 pv c2c4
info depth 14 score cp 17 time 540 nodes 1160475 nps 2149027 pv d2d4
info depth 15 score cp 24 time 871 nodes 1926464 nps 2211784 pv e2e4
info depth 16 score cp 25 time 1156 nodes 2584940 nps 2236107 pv e2e4
info depth 17 score cp 25 time 1599 nodes 3610243 nps 2257813 pv e2e4
info depth 18 score cp 25 time 2448 nodes 5753192 nps 2350160 pv e2e4
info depth 19 score cp 29 time 4100 nodes 9716191 nps 2369802 pv e2e4
bestmove e2e4
9716191 nodes 2369802 nps
```

```py
Elo   | 7.96 +- 5.25 (95%)
SPRT  | 10.0+0.10s Threads=1 Hash=1MB
LLR   | 2.97 (-2.94, 2.94) [0.00, 5.00]
Games | N: 7818 W: 2309 L: 2130 D: 3379
Penta | [196, 897, 1592, 980, 244]
https://gedas.pythonanywhere.com/test/1270/
```

Older test, same change:
```py
Elo   | 7.65 +- 5.17 (95%)
SPRT  | 10.0+0.10s Threads=1 Hash=1MB
LLR   | 3.00 (-2.94, 2.94) [0.00, 5.00]
Games | N: 8726 W: 2641 L: 2449 D: 3636
Penta | [259, 989, 1744, 1043, 328]
https://gedas.pythonanywhere.com/test/1176/
```

Older test, same change:
```py
Elo   | 4.24 +- 3.36 (95%)
SPRT  | 60.0+0.60s Threads=1 Hash=1MB
LLR   | 2.95 (-2.94, 2.94) [0.00, 5.00]
Games | N: 18990 W: 5613 L: 5381 D: 7996
Penta | [494, 2221, 3898, 2323, 559]
https://gedas.pythonanywhere.com/test/1177/
```

### 4.59

64 bit: 4094 bytes (-1)

MD5: 05a7ae7da673d4b07e5ebfa7fe3f077e

* Aspiration windows
* Remove More RFP if improving
* Remove Adjust NMP depth by beta distance to eval

```py
info lowerbound depth 1 score cp 26 time 0 nodes 18
info depth 1 score cp 26 time 0 nodes 38 pv b1c3
info depth 2 score cp 17 time 0 nodes 107 pv b1c3
info depth 3 score cp 26 time 0 nodes 236 pv b1c3
info depth 4 score cp 17 time 1 nodes 618 nps 618000 pv b1c3
info depth 5 score cp 12 time 1 nodes 1463 nps 1463000 pv d2d4
info depth 6 score cp 17 time 3 nodes 3704 nps 1234666 pv d2d4
info depth 7 score cp 8 time 6 nodes 6142 nps 1023666 pv d2d4
info depth 8 score cp 17 time 11 nodes 12272 nps 1115636 pv d2d4
info depth 9 score cp 10 time 18 nodes 21683 nps 1204611 pv d2d4
info depth 10 score cp 17 time 34 nodes 43372 nps 1275647 pv d2d4
info depth 11 score cp 25 time 55 nodes 87998 nps 1599963 pv d2d4
info depth 12 score cp 22 time 97 nodes 188963 nps 1948072 pv d2d4
info depth 13 score cp 12 time 224 nodes 526866 nps 2352080 pv d2d4
info depth 14 score cp 11 time 402 nodes 984565 nps 2449166 pv d2d4
info depth 15 score cp 30 time 1066 nodes 2760764 nps 2589834 pv e2e4
info depth 16 score cp 34 time 1607 nodes 4192972 nps 2609192 pv e2e4
info depth 17 score cp 29 time 2365 nodes 6196486 nps 2620078 pv e2e4
info depth 18 score cp 23 time 3550 nodes 9373973 nps 2640555 pv e2e4
info depth 19 score cp 25 time 5008 nodes 13281744 nps 2652105 pv e2e4
bestmove e2e4
13281744 nodes 2652105 nps
```

```py
Elo   | 10.16 +- 6.17 (95%)
SPRT  | 10.0+0.10s Threads=1 Hash=1MB
LLR   | 2.95 (-2.94, 2.94) [0.00, 5.00]
Games | N: 5852 W: 1774 L: 1603 D: 2475
Penta | [151, 660, 1187, 723, 205]
https://gedas.pythonanywhere.com/test/1291/
```

```py
Elo   | 13.03 +- 6.89 (95%)
SPRT  | 60.0+0.60s Threads=1 Hash=1MB
LLR   | 2.95 (-2.94, 2.94) [0.00, 5.00]
Games | N: 3974 W: 1137 L: 988 D: 1849
Penta | [73, 419, 878, 520, 97]
https://gedas.pythonanywhere.com/test/1292/
```

### 4.60

64 bit: 4094 bytes (=)

MD5: a221ebd3d9864e6ef56705201f72eb57

Aspiration window margin 16

```py
info lowerbound depth 1 score cp 18 time 0 nodes 5
info depth 1 score cp 26 time 1 nodes 25 nps 25000 pv b1c3
info depth 2 score cp 17 time 1 nodes 105 nps 105000 pv b1c3
info depth 3 score cp 25 time 1 nodes 243 nps 243000 pv b1c3
info depth 4 score cp 18 time 1 nodes 518 nps 518000 pv b1c3
info depth 5 score cp 12 time 2 nodes 1050 nps 525000 pv b1c3
info depth 6 score cp 16 time 3 nodes 2016 nps 672000 pv b1c3
info depth 7 score cp 12 time 8 nodes 7873 nps 984125 pv d2d4
info depth 8 score cp 17 time 17 nodes 17581 nps 1034176 pv d2d4
info depth 9 score cp 10 time 32 nodes 39206 nps 1225187 pv d2d4
info depth 10 score cp 17 time 45 nodes 61120 nps 1358222 pv d2d4
info depth 11 score cp 11 time 91 nodes 170746 nps 1876329 pv g1f3
info depth 12 score cp 19 time 153 nodes 318484 nps 2081594 pv g1f3
info depth 13 score cp 12 time 364 nodes 848197 nps 2330211 pv b1c3
info lowerbound depth 14 score cp 28 time 571 nodes 1388924 nps 2432441
info depth 14 score cp 21 time 643 nodes 1572278 nps 2445222 pv e2e4
info depth 15 score cp 22 time 878 nodes 2191806 nps 2496362 pv e2e4
info depth 16 score cp 30 time 1283 nodes 3249082 nps 2532409 pv e2e4
info upperbound depth 17 score cp 14 time 2608 nodes 6794425 nps 2605224
info depth 17 score cp 28 time 2808 nodes 7306694 nps 2602099 pv e2e4
info depth 18 score cp 28 time 3517 nodes 9099047 nps 2587161 pv e2e4
info depth 19 score cp 28 time 4691 nodes 12254833 nps 2612413 pv e2e4
bestmove e2e4
12254833 nodes 2612413 nps
```

```py
Elo   | 7.66 +- 5.21 (95%)
SPRT  | 10.0+0.10s Threads=1 Hash=1MB
LLR   | 2.95 (-2.94, 2.94) [0.00, 5.00]
Games | N: 8528 W: 2592 L: 2404 D: 3532
Penta | [257, 964, 1674, 1072, 297]
https://gedas.pythonanywhere.com/test/1293/
```

```py
Elo   | 6.35 +- 13.86 (95%)
SPRT  | 60.0+0.60s Threads=1 Hash=1MB
LLR   | 0.31 (-2.94, 2.94) [0.00, 5.00]
Games | N: 1040 W: 279 L: 260 D: 501
Penta | [19, 127, 219, 126, 29]
https://gedas.pythonanywhere.com/test/1297/
```

### 4.61

64 bit: 4096 bytes (+2)

MD5: f0acee1a7d8a1565ae5f0304ad93fef3

More RFP if improving

```py
info lowerbound depth 1 score cp 18 time 1 nodes 5 nps 5000
info depth 1 score cp 26 time 1 nodes 25 nps 25000 pv b1c3
info depth 2 score cp 17 time 1 nodes 105 nps 105000 pv b1c3
info depth 3 score cp 25 time 1 nodes 241 nps 241000 pv b1c3
info depth 4 score cp 18 time 1 nodes 496 nps 496000 pv b1c3
info depth 5 score cp 12 time 2 nodes 1087 nps 543500 pv b1c3
info depth 6 score cp 15 time 2 nodes 2067 nps 1033500 pv b1c3
info depth 7 score cp 12 time 6 nodes 6316 nps 1052666 pv b1c3
info depth 8 score cp 17 time 10 nodes 11387 nps 1138700 pv b1c3
info depth 9 score cp 10 time 23 nodes 27765 nps 1207173 pv b1c3
info lowerbound depth 10 score cp 26 time 32 nodes 43264 nps 1352000
info depth 10 score cp 29 time 38 nodes 54159 nps 1425236 pv b1c3
info upperbound depth 11 score cp 13 time 51 nodes 78623 nps 1541627
info depth 11 score cp 22 time 60 nodes 96939 nps 1615650 pv b1c3
info depth 12 score cp 16 time 103 nodes 200587 nps 1947446 pv b1c3
info depth 13 score cp 16 time 246 nodes 557561 nps 2266508 pv c2c4
info depth 14 score cp 17 time 434 nodes 1037527 nps 2390615 pv b1c3
info depth 15 score cp 28 time 697 nodes 1726750 nps 2477403 pv e2e4
info depth 16 score cp 25 time 968 nodes 2433471 nps 2513916 pv e2e4
info depth 17 score cp 29 time 1527 nodes 3959886 nps 2593245 pv e2e4
info depth 18 score cp 29 time 2155 nodes 5563615 nps 2581723 pv e2e4
info upperbound depth 19 score cp 13 time 5559 nodes 14381395 nps 2587047
info depth 19 score cp 21 time 6144 nodes 15911196 nps 2589712 pv e2e4
bestmove e2e4
15911196 nodes 2589712 nps
```

```py
Elo   | 8.44 +- 5.50 (95%)
SPRT  | 10.0+0.10s Threads=1 Hash=1MB
LLR   | 2.95 (-2.94, 2.94) [0.00, 5.00]
Games | N: 7372 W: 2255 L: 2076 D: 3041
Penta | [200, 840, 1467, 939, 240]
https://gedas.pythonanywhere.com/test/1301/
```

```py
Elo   | 4.72 +- 3.60 (95%)
SPRT  | 60.0+0.60s Threads=1 Hash=1MB
LLR   | 2.97 (-2.94, 2.94) [0.00, 5.00]
Games | N: 14638 W: 4007 L: 3808 D: 6823
Penta | [271, 1743, 3154, 1818, 333]
https://gedas.pythonanywhere.com/test/1303/
```

### 4.62

64 bit: 4096 bytes (=)

MD5: 5f075b47f8da6b6532a84d96728e29a5

NMP base depth reduction 4

```py
info lowerbound depth 1 score cp 18 time 0 nodes 5
info depth 1 score cp 26 time 0 nodes 25 pv b1c3
info depth 2 score cp 17 time 0 nodes 105 pv b1c3
info depth 3 score cp 25 time 0 nodes 241 pv b1c3
info depth 4 score cp 18 time 0 nodes 496 pv b1c3
info depth 5 score cp 12 time 1 nodes 1087 nps 1087000 pv b1c3
info depth 6 score cp 15 time 2 nodes 2001 nps 1000500 pv b1c3
info depth 7 score cp 12 time 5 nodes 6137 nps 1227400 pv b1c3
info depth 8 score cp 17 time 9 nodes 10499 nps 1166555 pv b1c3
info depth 9 score cp 10 time 21 nodes 25829 nps 1229952 pv b1c3
info depth 10 score cp 17 time 31 nodes 41182 nps 1328451 pv b1c3
info depth 11 score cp 25 time 53 nodes 80867 nps 1525792 pv b1c3
info depth 12 score cp 17 time 124 nodes 228611 nps 1843637 pv c2c4
info depth 13 score cp 21 time 172 nodes 329495 nps 1915668 pv c2c4
info depth 14 score cp 20 time 332 nodes 706151 nps 2126960 pv e2e4
info depth 15 score cp 17 time 598 nodes 1373724 nps 2297197 pv e2e4
info depth 16 score cp 25 time 826 nodes 1945568 nps 2355409 pv e2e4
info depth 17 score cp 30 time 1315 nodes 3186430 nps 2423140 pv e2e4
info depth 18 score cp 34 time 2104 nodes 5183545 nps 2463662 pv e2e4
info depth 19 score cp 23 time 3045 nodes 7542173 nps 2476904 pv e2e4
bestmove e2e4
7542173 nodes 2476904 nps
```

```py
Elo   | 4.89 +- 3.75 (95%)
SPRT  | 10.0+0.10s Threads=1 Hash=1MB
LLR   | 3.00 (-2.94, 2.94) [0.00, 5.00]
Games | N: 15920 W: 4697 L: 4473 D: 6750
Penta | [468, 1823, 3181, 1993, 495]
https://gedas.pythonanywhere.com/test/1309/
```

```py
Elo   | 5.92 +- 4.23 (95%)
SPRT  | 60.0+0.60s Threads=1 Hash=1MB
LLR   | 2.95 (-2.94, 2.94) [0.00, 5.00]
Games | N: 10328 W: 2810 L: 2634 D: 4884
Penta | [188, 1179, 2289, 1285, 223]
https://gedas.pythonanywhere.com/test/1310/
```

### 4.63

64 bit: 4086 bytes (-10)

MD5: 3c9222aee8a505f1f30a8be15e5af073

More ordering permutations

```py
info depth 1 score cp 16 lowerbound time 0 nodes 5
info depth 1 score cp 26 time 0 nodes 25 pv b1c3
info depth 2 score cp 17 time 0 nodes 105 pv b1c3
info depth 3 score cp 25 time 1 nodes 241 nps 241000 pv b1c3
info depth 4 score cp 18 time 1 nodes 496 nps 496000 pv b1c3
info depth 5 score cp 13 time 2 nodes 1233 nps 616500 pv d2d4
info depth 6 score cp 17 time 4 nodes 3271 nps 817750 pv b1c3
info depth 7 score cp 12 time 5 nodes 5589 nps 1117800 pv b1c3
info depth 8 score cp 17 time 10 nodes 12479 nps 1247900 pv b1c3
info depth 9 score cp 1 upperbound time 18 nodes 35994 nps 1999666
info depth 9 score cp -2 time 20 nodes 40994 nps 2049700 pv b1c3
info depth 10 score cp 14 lowerbound time 26 nodes 54897 nps 2111423
info depth 10 score cp 16 time 32 nodes 64222 nps 2006937 pv e2e4
info depth 11 score cp 11 time 49 nodes 118416 nps 2416653 pv g1f3
info depth 12 score cp 16 time 82 nodes 183220 nps 2234390 pv g1f3
info depth 13 score cp 16 time 195 nodes 485607 nps 2490292 pv d2d4
info depth 14 score cp 16 time 282 nodes 709275 nps 2515159 pv d2d4
info depth 15 score cp 23 time 553 nodes 1384822 nps 2504198 pv e2e4
info depth 16 score cp 23 time 719 nodes 1816909 nps 2526994 pv e2e4
info depth 17 score cp 24 time 1111 nodes 2659492 nps 2393782 pv e2e4
info depth 18 score cp 28 time 1776 nodes 4084439 nps 2299796 pv e2e4
info depth 19 score cp 29 time 3268 nodes 8058826 nps 2465981 pv e2e4
bestmove e2e4
8058826 nodes 2465981 nps
```

```py
Elo   | 0.99 +- 3.51 (95%)
SPRT  | 10.0+0.10s Threads=1 Hash=1MB
LLR   | 2.96 (-2.94, 2.94) [-5.00, 0.00]
Games | N: 18254 W: 5391 L: 5339 D: 7524
Penta | [574, 2101, 3733, 2137, 582]
https://gedas.pythonanywhere.com/test/1318/
```

### 4.64

64 bit: 4086 bytes (=)

MD5: a2db932f046c014a2d40bccb65233f58

Tune search parameters

```py
info depth 1 score cp 15 lowerbound time 0 nodes 5
info depth 1 score cp 26 time 0 nodes 25 pv b1c3
info depth 2 score cp 17 time 0 nodes 104 pv b1c3
info depth 3 score cp 25 time 0 nodes 237 pv b1c3
info depth 4 score cp 18 time 1 nodes 480 nps 480000 pv b1c3
info depth 5 score cp 12 time 1 nodes 1047 nps 1047000 pv b1c3
info depth 6 score cp 15 time 2 nodes 1890 nps 945000 pv b1c3
info depth 7 score cp 12 time 5 nodes 5251 nps 1050200 pv b1c3
info depth 8 score cp 17 time 7 nodes 8835 nps 1262142 pv b1c3
info depth 9 score cp 10 time 15 nodes 17626 nps 1175066 pv b1c3
info depth 10 score cp 17 time 26 nodes 35562 nps 1367769 pv b1c3
info depth 11 score cp 13 time 46 nodes 71854 nps 1562043 pv b1c3
info depth 12 score cp 10 time 91 nodes 171965 nps 1889725 pv b1c3
info depth 13 score cp 16 time 159 nodes 344297 nps 2165389 pv g1f3
info depth 14 score cp 18 time 217 nodes 484708 nps 2233677 pv g1f3
info depth 15 score cp 11 time 450 nodes 1082663 nps 2405917 pv g1f3
info depth 16 score cp 26 lowerbound time 758 nodes 1890589 nps 2494180
info depth 16 score cp 27 time 873 nodes 2197828 nps 2517557 pv e2e4
info depth 17 score cp 29 time 1154 nodes 2923855 nps 2533669 pv e2e4
info depth 18 score cp 21 time 1590 nodes 4082510 nps 2567616 pv e2e4
info depth 19 score cp 21 time 2198 nodes 5662842 nps 2576361 pv e2e4
info depth 20 score cp 36 lowerbound time 5266 nodes 13771935 nps 2615255
info depth 20 score cp 27 time 5584 nodes 14621729 nps 2618504 pv e2e4
bestmove e2e4
14621729 nodes 2618504 nps
```

```py
Elo   | 11.13 +- 6.47 (95%)
SPRT  | 10.0+0.10s Threads=1 Hash=1MB
LLR   | 2.96 (-2.94, 2.94) [0.00, 5.00]
Games | N: 5182 W: 1556 L: 1390 D: 2236
Penta | [135, 554, 1082, 650, 170]
https://gedas.pythonanywhere.com/test/1328/
```

```py
Elo   | 9.56 +- 5.72 (95%)
SPRT  | 60.0+0.60s Threads=1 Hash=1MB
LLR   | 2.97 (-2.94, 2.94) [0.00, 5.00]
Games | N: 5670 W: 1552 L: 1396 D: 2722
Penta | [88, 662, 1221, 734, 130]
https://gedas.pythonanywhere.com/test/1331/
```

### 4.65

64 bit: 4094 bytes (+8)

MD5: 42295baf13e8bdfe28bda682e68f37d7

LMR more for bad histories

```py
info depth 1 score cp 15 lowerbound time 0 nodes 5
info depth 1 score cp 26 time 0 nodes 25 pv b1c3
info depth 2 score cp 17 time 0 nodes 104 pv b1c3
info depth 3 score cp 25 time 0 nodes 237 pv b1c3
info depth 4 score cp 18 time 0 nodes 480 pv b1c3
info depth 5 score cp 12 time 1 nodes 1047 nps 1047000 pv b1c3
info depth 6 score cp 15 time 2 nodes 1890 nps 945000 pv b1c3
info depth 7 score cp 12 time 5 nodes 5251 nps 1050200 pv b1c3
info depth 8 score cp 17 time 8 nodes 8835 nps 1104375 pv b1c3
info depth 9 score cp 10 time 15 nodes 17626 nps 1175066 pv b1c3
info depth 10 score cp 17 time 28 nodes 35562 nps 1270071 pv b1c3
info depth 11 score cp 13 time 49 nodes 71854 nps 1466408 pv b1c3
info depth 12 score cp 10 time 96 nodes 171965 nps 1791302 pv b1c3
info depth 13 score cp 16 time 169 nodes 344297 nps 2037260 pv g1f3
info depth 14 score cp 18 time 230 nodes 484707 nps 2107421 pv g1f3
info depth 15 score cp 16 time 384 nodes 875193 nps 2279148 pv g1f3
info depth 16 score cp 31 lowerbound time 759 nodes 1854903 nps 2443877
info depth 16 score cp 25 time 832 nodes 2035823 nps 2446902 pv e2e4
info depth 17 score cp 30 time 1003 nodes 2480131 nps 2472712 pv e2e4
info depth 18 score cp 28 time 1258 nodes 3170835 nps 2520536 pv e2e4
info depth 19 score cp 26 time 2368 nodes 6177607 nps 2608786 pv e2e4
info depth 20 score cp 28 time 3315 nodes 8692738 nps 2622243 pv e2e4
bestmove e2e4
8692738 nodes 2622243 nps
```

```py
Elo   | 9.41 +- 5.83 (95%)
SPRT  | 10.0+0.10s Threads=1 Hash=1MB
LLR   | 2.95 (-2.94, 2.94) [0.00, 5.00]
Games | N: 6242 W: 1861 L: 1692 D: 2689
Penta | [146, 724, 1249, 819, 183]
https://gedas.pythonanywhere.com/test/1342/
```

```py
Elo   | 5.33 +- 3.89 (95%)
SPRT  | 60.0+0.60s Threads=1 Hash=1MB
LLR   | 2.96 (-2.94, 2.94) [0.00, 5.00]
Games | N: 11530 W: 3037 L: 2860 D: 5633
Penta | [185, 1296, 2672, 1381, 231]
https://gedas.pythonanywhere.com/test/1345/
```

### 4.66

64 bit: 4094 bytes (=)

MD5: 06835c97083284149fe5a75a94de748e

LMR from 3 evaluated moves

```py
info depth 1 score cp 15 lowerbound time 0 nodes 5
info depth 1 score cp 26 time 0 nodes 25 pv b1c3
info depth 2 score cp 17 time 0 nodes 101 pv b1c3
info depth 3 score cp 25 time 0 nodes 228 pv b1c3
info depth 4 score cp 25 time 0 nodes 409 pv b1c3
info depth 5 score cp 22 time 1 nodes 652 nps 652000 pv b1c3
info depth 6 score cp 12 time 1 nodes 1701 nps 1701000 pv b1c3
info depth 7 score cp 12 time 4 nodes 4491 nps 1122750 pv b1c3
info depth 8 score cp 17 time 6 nodes 6924 nps 1154000 pv b1c3
info depth 9 score cp 10 time 11 nodes 13261 nps 1205545 pv b1c3
info depth 10 score cp 17 time 21 nodes 25764 nps 1226857 pv b1c3
info depth 11 score cp 13 time 39 nodes 56887 nps 1458641 pv b1c3
info depth 12 score cp 28 lowerbound time 53 nodes 81385 nps 1535566
info depth 12 score cp 10 time 79 nodes 137437 nps 1739708 pv b1c3
info depth 13 score cp 19 time 102 nodes 193855 nps 1900539 pv b1c3
info depth 14 score cp 8 time 227 nodes 507076 nps 2233814 pv b1c3
info depth 15 score cp 23 lowerbound time 322 nodes 747955 nps 2322841
info depth 15 score cp 25 time 350 nodes 815953 nps 2331294 pv e2e4
info depth 16 score cp 21 time 417 nodes 982559 nps 2356256 pv e2e4
info depth 17 score cp 20 time 528 nodes 1262435 nps 2390975 pv e2e4
info depth 18 score cp 25 time 909 nodes 2244492 nps 2469188 pv e2e4
info depth 19 score cp 19 time 1278 nodes 3177151 nps 2486033 pv e2e4
info depth 20 score cp 34 lowerbound time 1758 nodes 4447385 nps 2529798
info depth 20 score cp 38 time 2309 nodes 5932238 nps 2569180 pv e2e4
info depth 21 score cp 38 time 2770 nodes 7147620 nps 2580368 pv e2e4
info depth 22 score cp 35 time 3934 nodes 10098456 nps 2566968 pv e2e4
bestmove e2e4
10098456 nodes 2566968 nps
```

```py
Elo   | 5.89 +- 4.26 (95%)
SPRT  | 10.0+0.10s Threads=1 Hash=1MB
LLR   | 3.00 (-2.94, 2.94) [0.00, 5.00]
Games | N: 11678 W: 3355 L: 3157 D: 5166
Penta | [301, 1346, 2369, 1500, 323]
https://gedas.pythonanywhere.com/test/1369/
```

```py
Elo   | 2.70 +- 2.15 (95%)
SPRT  | 60.0+0.60s Threads=1 Hash=1MB
LLR   | 2.98 (-2.94, 2.94) [0.00, 5.00]
Games | N: 38046 W: 9770 L: 9474 D: 18802
Penta | [613, 4536, 8499, 4692, 683]
https://gedas.pythonanywhere.com/test/1371/
```

### 4.67

64 bit: 4094 bytes (=)

MD5: 3adabc585a7680030beedc88bde2e936

LMR from depth 2

```py
info depth 1 score cp 15 lowerbound time 1 nodes 5 nps 5000
info depth 1 score cp 26 time 1 nodes 25 nps 25000 pv b1c3
info depth 2 score cp 17 time 1 nodes 118 nps 118000 pv b1c3
info depth 3 score cp 25 time 1 nodes 242 nps 242000 pv b1c3
info depth 4 score cp 18 time 1 nodes 471 nps 471000 pv b1c3
info depth 5 score cp 12 time 1 nodes 887 nps 887000 pv b1c3
info depth 6 score cp 12 time 2 nodes 1508 nps 754000 pv b1c3
info depth 7 score cp 12 time 5 nodes 4094 nps 818800 pv b1c3
info depth 8 score cp 17 time 8 nodes 7024 nps 878000 pv b1c3
info depth 9 score cp 10 time 15 nodes 14809 nps 987266 pv b1c3
info depth 10 score cp 17 time 22 nodes 23733 nps 1078772 pv b1c3
info depth 11 score cp 13 time 35 nodes 42106 nps 1203028 pv b1c3
info depth 12 score cp 22 time 67 nodes 94973 nps 1417507 pv b1c3
info depth 13 score cp 16 time 208 nodes 366473 nps 1761889 pv c2c4
info depth 14 score cp 8 time 356 nodes 659230 nps 1851769 pv c2c4
info depth 15 score cp 14 time 423 nodes 790474 nps 1868732 pv c2c4
info depth 16 score cp 14 time 632 nodes 1207434 nps 1910496 pv c2c4
info depth 17 score cp 20 time 936 nodes 1825093 nps 1949885 pv e2e4
info depth 18 score cp 31 time 1247 nodes 2465249 nps 1976943 pv e2e4
info depth 19 score cp 29 time 1544 nodes 3097795 nps 2006343 pv e2e4
info depth 20 score cp 29 time 2032 nodes 4121914 nps 2028500 pv e2e4
info depth 21 score cp 29 time 3120 nodes 6501647 nps 2083861 pv e2e4
info depth 22 score cp 25 time 5136 nodes 11007709 nps 2143245 pv e2e4
bestmove e2e4
11007709 nodes 2143245 nps
```

```py
Elo   | 9.55 +- 5.78 (95%)
SPRT  | 10.0+0.10s Threads=1 Hash=1MB
LLR   | 3.04 (-2.94, 2.94) [0.00, 5.00]
Games | N: 6262 W: 1788 L: 1616 D: 2858
Penta | [147, 708, 1278, 822, 176]
https://gedas.pythonanywhere.com/test/1384/
```

```py
Elo   | 16.04 +- 7.65 (95%)
SPRT  | 60.0+0.60s Threads=1 Hash=1MB
LLR   | 2.95 (-2.94, 2.94) [0.00, 5.00]
Games | N: 3078 W: 849 L: 707 D: 1522
Penta | [46, 319, 693, 409, 72]
https://gedas.pythonanywhere.com/test/1385/
```

### 5.0

64 bit: 4094 bytes (=)

MD5: 3adabc585a7680030beedc88bde2e936

Equivalent to 4.67

10+0.01:
```py
Score of 4kc-5.0 vs 4ku-3.0: 1835 - 1450 - 1715  [0.538] 5000
...      4kc-5.0 playing White: 1219 - 443 - 838  [0.655] 2500
...      4kc-5.0 playing Black: 616 - 1007 - 877  [0.422] 2500
...      White vs Black: 2226 - 1059 - 1715  [0.617] 5000
Elo difference: 26.8 +/- 7.8, LOS: 100.0 %, DrawRatio: 34.3 %

Score of 4kc-5.0 vs 4ku-3.1: 1790 - 1647 - 1563  [0.514] 5000
...      4kc-5.0 playing White: 1196 - 494 - 811  [0.640] 2501
...      4kc-5.0 playing Black: 594 - 1153 - 752  [0.388] 2499
...      White vs Black: 2349 - 1088 - 1563  [0.626] 5000
Elo difference: 9.9 +/- 8.0, LOS: 99.3 %, DrawRatio: 31.3 %

Score of 4kc-5.0 vs 4ku-4.0: 2243 - 6743 - 4014  [0.327] 13000
...      4kc-5.0 playing White: 1669 - 2459 - 2373  [0.439] 6501
...      4kc-5.0 playing Black: 574 - 4284 - 1641  [0.215] 6499
...      White vs Black: 5953 - 3033 - 4014  [0.612] 13000
Elo difference: -125.4 +/- 5.1, LOS: 0.0 %, DrawRatio: 30.9 %
```

60+0.06:
```py
Score of 4kc-5.0 vs 4ku-3.0: 2455 - 1231 - 2314  [0.602] 6000
...      4kc-5.0 playing White: 1709 - 271 - 1020  [0.740] 3000
...      4kc-5.0 playing Black: 746 - 960 - 1294  [0.464] 3000
...      White vs Black: 2669 - 1017 - 2314  [0.638] 6000
Elo difference: 71.9 +/- 6.9, LOS: 100.0 %, DrawRatio: 38.6 %
```

### 5.1

64 bit: 4091 bytes (-3)

MD5: 2e3361192fe1e63c876c689dad2ba000

Smaller PSTs


### 5.2

64 bit: 4088 bytes (-3)

MD5: 6b4ee2296f123e9dc263534d05a61482

Smaller loader (thanks to sqrmax)

### 5.3

64 bit: 4088 bytes (=)

MD5: 0f6b895ceb311ffce6681c54daf7e7a9

LMR from depth 3

```py
info depth 1 score cp 15 lowerbound time 0 nodes 5
info depth 1 score cp 26 time 0 nodes 25 pv b1c3
info depth 2 score cp 17 time 0 nodes 118 pv b1c3
info depth 3 score cp 25 time 0 nodes 293 pv b1c3
info depth 4 score cp 17 time 1 nodes 622 nps 622000 pv b1c3
info depth 5 score cp 8 time 1 nodes 961 nps 961000 pv b1c3
info depth 6 score cp 12 time 2 nodes 2307 nps 1153500 pv b1c3
info depth 7 score cp 12 time 5 nodes 5136 nps 1027200 pv b1c3
info depth 8 score cp 17 time 8 nodes 7558 nps 944750 pv b1c3
info depth 9 score cp 18 time 15 nodes 16566 nps 1104400 pv b1c3
info depth 10 score cp 17 time 32 nodes 38241 nps 1195031 pv b1c3
info depth 11 score cp 25 time 46 nodes 59158 nps 1286043 pv b1c3
info depth 12 score cp 27 time 71 nodes 103511 nps 1457901 pv b1c3
info depth 13 score cp 12 upperbound time 108 nodes 167655 nps 1552361
info depth 13 score cp 13 time 139 nodes 228555 nps 1644280 pv b1c3
info depth 14 score cp 11 time 207 nodes 357042 nps 1724840 pv b1c3
info depth 15 score cp 23 time 360 nodes 659799 nps 1832775 pv e2e4
info depth 16 score cp 24 time 459 nodes 849980 nps 1851808 pv e2e4
info depth 17 score cp 29 time 756 nodes 1467556 nps 1941211 pv e2e4
info depth 18 score cp 14 upperbound time 1448 nodes 2873682 nps 1984587
info depth 18 score cp 21 time 1536 nodes 3048935 nps 1984983 pv e2e4
info depth 19 score cp 35 time 2095 nodes 4214595 nps 2011739 pv e2e4
info depth 20 score cp 35 time 2436 nodes 4902049 nps 2012335 pv e2e4
info depth 21 score cp 29 time 3026 nodes 6117194 nps 2021544 pv e2e4
info depth 22 score cp 37 time 6499 nodes 13024915 nps 2004141 pv e2e4
bestmove e2e4
13024915 nodes 2003833 nps
```

```py
Elo   | -0.00 +- 2.66 (95%)
SPRT  | 10.0+0.10s Threads=1 Hash=1MB
LLR   | -3.06 (-2.94, 2.94) [0.00, 5.00]
Games | N: 29576 W: 8019 L: 8019 D: 13538
Penta | [765, 3624, 6021, 3602, 776]
https://gedas.pythonanywhere.com/test/1391/
```

```py
Elo   | 4.22 +- 3.28 (95%)
SPRT  | 60.0+0.60s Threads=1 Hash=1MB
LLR   | 2.97 (-2.94, 2.94) [0.00, 5.00]
Games | N: 16618 W: 4283 L: 4081 D: 8254
Penta | [287, 1950, 3628, 2162, 282]
https://gedas.pythonanywhere.com/test/1393/
```

### 5.4

64 bit: 4095 bytes (+7)

MD5: df57886db53c4253079d3603b102d343

No LMR for positive move scores

```py
info depth 1 score cp 15 lowerbound time 0 nodes 5
info depth 1 score cp 26 time 0 nodes 25 pv b1c3
info depth 2 score cp 17 time 1 nodes 118 nps 118000 pv b1c3
info depth 3 score cp 25 time 1 nodes 293 nps 293000 pv b1c3
info depth 4 score cp 17 time 1 nodes 622 nps 622000 pv b1c3
info depth 5 score cp 8 time 2 nodes 961 nps 480500 pv b1c3
info depth 6 score cp 15 time 7 nodes 3632 nps 518857 pv d2d4
info depth 7 score cp 12 time 17 nodes 7337 nps 431588 pv d2d4
info depth 8 score cp 17 time 36 nodes 16066 nps 446277 pv d2d4
info depth 9 score cp 7 time 56 nodes 26919 nps 480696 pv d2d4
info depth 10 score cp 17 time 92 nodes 47206 nps 513108 pv d2d4
info depth 11 score cp 13 time 137 nodes 77496 nps 565664 pv d2d4
info depth 12 score cp 28 lowerbound time 212 nodes 118312 nps 558075
info depth 12 score cp 17 time 317 nodes 204422 nps 644864 pv d2d4
info depth 13 score cp 16 time 472 nodes 334259 nps 708175 pv d2d4
info depth 14 score cp 12 time 711 nodes 516538 nps 726495 pv d2d4
info depth 15 score cp 12 time 1108 nodes 850789 nps 767860 pv d2d4
info depth 16 score cp 16 time 2045 nodes 1641673 nps 802774 pv c2c4
info depth 17 score cp 31 lowerbound time 3268 nodes 2683004 nps 820992
info depth 17 score cp 29 time 3390 nodes 2786572 nps 821997 pv e2e4
info depth 18 score cp 23 time 4069 nodes 3429420 nps 842816 pv e2e4
info depth 19 score cp 23 time 4961 nodes 4165542 nps 839657 pv e2e4
info depth 20 score cp 23 time 5691 nodes 4830531 nps 848801 pv e2e4
info depth 21 score cp 22 time 7568 nodes 6520076 nps 861532 pv e2e4
info depth 22 score cp 29 time 11207 nodes 9835986 nps 877664 pv e2e4
bestmove e2e4
9835986 nodes 877664 nps
```

```py
Elo   | 4.17 +- 3.30 (95%)
SPRT  | 10.0+0.10s Threads=1 Hash=1MB
LLR   | 2.96 (-2.94, 2.94) [0.00, 5.00]
Games | N: 19658 W: 5541 L: 5305 D: 8812
Penta | [512, 2327, 3958, 2477, 555]
https://gedas.pythonanywhere.com/test/1398/
```

```py
Elo   | 5.26 +- 3.85 (95%)
SPRT  | 60.0+0.60s Threads=1 Hash=1MB
LLR   | 2.95 (-2.94, 2.94) [0.00, 5.00]
Games | N: 11492 W: 3014 L: 2840 D: 5638
Penta | [163, 1323, 2645, 1407, 208]
https://gedas.pythonanywhere.com/test/1402/
```

### 5.5

64 bit: 4086 bytes (-9)

MD5: 130654e1a7190487a91d275921e23059

No moves check for LMR

```py
info depth 1 score cp 15 lowerbound time 0 nodes 5
info depth 1 score cp 26 time 0 nodes 25 pv b1c3
info depth 2 score cp 17 time 1 nodes 118 nps 118000 pv b1c3
info depth 3 score cp 25 time 1 nodes 293 nps 293000 pv b1c3
info depth 4 score cp 17 time 1 nodes 622 nps 622000 pv b1c3
info depth 5 score cp 8 time 2 nodes 961 nps 480500 pv b1c3
info depth 6 score cp 15 time 7 nodes 3632 nps 518857 pv d2d4
info depth 7 score cp 12 time 14 nodes 7337 nps 524071 pv d2d4
info depth 8 score cp 17 time 54 nodes 16066 nps 297518 pv d2d4
info depth 9 score cp 7 time 101 nodes 26907 nps 266405 pv d2d4
info depth 10 score cp 17 time 185 nodes 46962 nps 253848 pv d2d4
info depth 11 score cp 13 time 252 nodes 74933 nps 297353 pv d2d4
info depth 12 score cp 17 time 344 nodes 147265 nps 428095 pv d2d4
info depth 13 score cp 20 time 841 nodes 544411 nps 647337 pv c2c4
info depth 14 score cp 13 time 1267 nodes 793493 nps 626277 pv c2c4
info depth 15 score cp 18 time 1904 nodes 1198112 nps 629260 pv c2c4
info depth 16 score cp 31 time 2613 nodes 1794816 nps 686879 pv e2e4
info depth 17 score cp 16 upperbound time 3046 nodes 2167559 nps 711608
info depth 17 score cp 32 time 3709 nodes 2732770 nps 736794 pv e2e4
info depth 18 score cp 17 upperbound time 4334 nodes 3256883 nps 751472
info depth 18 score cp 29 time 4614 nodes 3499401 nps 758431 pv e2e4
info depth 19 score cp 21 time 5669 nodes 4431327 nps 781677 pv e2e4
info depth 20 score cp 23 time 7567 nodes 6030642 nps 796966 pv e2e4
info depth 21 score cp 32 time 9998 nodes 8096453 nps 809807 pv e2e4
info depth 22 score cp 23 time 15136 nodes 12556507 nps 829578 pv e2e4
bestmove e2e4
12556507 nodes 829578 nps
```

```py
Elo   | 5.36 +- 5.50 (95%)
SPRT  | 10.0+0.10s Threads=1 Hash=1MB
LLR   | 2.99 (-2.94, 2.94) [-5.00, 0.00]
Games | N: 6876 W: 1924 L: 1818 D: 3134
Penta | [169, 795, 1421, 867, 186]
https://gedas.pythonanywhere.com/test/1406/
```

```py
Elo   | 4.68 +- 5.02 (95%)
SPRT  | 60.0+0.60s Threads=1 Hash=1MB
LLR   | 2.96 (-2.94, 2.94) [-5.00, 0.00]
Games | N: 7128 W: 1839 L: 1743 D: 3546
Penta | [121, 831, 1580, 895, 137]
https://gedas.pythonanywhere.com/test/1407/
```

### 5.6

64 bit: 4084 bytes (-2)

MD5: 45d27097649cf7bd3113746674084565

Bit based blackbox

### 5.7

64 bit: 4082 bytes (-2)

MD5: b8c78a8a5eb8814b1c1bdda894c0ac21

Loader size peephole optimization (implemented by sqrmax)

### 5.8

64 bit: 4095 bytes (+13)

MD5: 29a0472f7900be3073f24a075539d1eb

Protected pawn evaluation

```py
info depth 1 score cp 14 time 0 nodes 13 pv e2e3
info depth 2 score cp 17 time 0 nodes 118 pv b1c3
info depth 3 score cp 26 time 1 nodes 263 nps 263000 pv b1c3
info depth 4 score cp 16 time 5 nodes 493 nps 98600 pv b1c3
info depth 5 score cp 10 time 7 nodes 878 nps 125428 pv b1c3
info depth 6 score cp 17 time 8 nodes 1625 nps 203125 pv b1c3
info depth 7 score cp 15 time 12 nodes 4831 nps 402583 pv d2d3
info depth 8 score cp 17 time 15 nodes 7442 nps 496133 pv d2d3
info depth 9 score cp 22 time 35 nodes 21518 nps 614800 pv g1f3
info depth 10 score cp 15 time 59 nodes 45304 nps 767864 pv g1f3
info depth 11 score cp 17 time 117 nodes 110415 nps 943717 pv d2d4
info depth 12 score cp 17 time 189 nodes 205789 nps 1088830 pv d2d4
info depth 13 score cp 10 time 462 nodes 583508 nps 1263004 pv c2c4
info depth 14 score cp 11 time 555 nodes 711813 nps 1282545 pv c2c4
info depth 15 score cp 20 time 1275 nodes 1715012 nps 1345107 pv e2e4
info depth 16 score cp 22 time 1482 nodes 2020643 nps 1363456 pv e2e4
info depth 17 score cp 22 time 1798 nodes 2474224 nps 1376097 pv e2e4
info depth 18 score cp 22 time 2143 nodes 2973716 nps 1387641 pv e2e4
info depth 19 score cp 33 time 3447 nodes 4837025 nps 1403256 pv e2e4
info depth 20 score cp 18 upperbound time 5688 nodes 7952978 nps 1398202
info depth 20 score cp 31 time 7479 nodes 10646595 nps 1423531 pv e2e4
info depth 21 score cp 26 time 8695 nodes 12465878 nps 1433683 pv e2e4
info depth 22 score cp 32 time 10971 nodes 15935342 nps 1452496 pv e2e4
bestmove e2e4
15935342 nodes 1452496 nps
```

```py
Elo   | 20.84 +- 9.26 (95%)
SPRT  | 10.0+0.10s Threads=1 Hash=1MB
LLR   | 2.97 (-2.94, 2.94) [0.00, 5.00]
Games | N: 2604 W: 795 L: 639 D: 1170
Penta | [55, 285, 511, 351, 100]
https://gedas.pythonanywhere.com/test/1429/
```

```py
Elo   | 14.03 +- 7.13 (95%)
SPRT  | 60.0+0.60s Threads=1 Hash=1MB
LLR   | 2.96 (-2.94, 2.94) [0.00, 5.00]
Games | N: 3618 W: 1030 L: 884 D: 1704
Penta | [56, 398, 779, 496, 80]
https://gedas.pythonanywhere.com/test/1430/
```

### 5.9

64 bit: 4087 bytes (-8)

MD5: 26c8204fd3e8d818449056c2ef783383

Smaller flip_pos (idea by sqrmax)

```py
info depth 1 score cp 14 time 0 nodes 13 pv e2e3
info depth 2 score cp 17 time 0 nodes 118 pv b1c3
info depth 3 score cp 26 time 0 nodes 263 pv b1c3
info depth 4 score cp 16 time 1 nodes 493 nps 493000 pv b1c3
info depth 5 score cp 10 time 1 nodes 878 nps 878000 pv b1c3
info depth 6 score cp 17 time 2 nodes 1625 nps 812500 pv b1c3
info depth 7 score cp 15 time 4 nodes 4831 nps 1207750 pv d2d3
info depth 8 score cp 17 time 7 nodes 7442 nps 1063142 pv d2d3
info depth 9 score cp 22 time 18 nodes 21518 nps 1195444 pv g1f3
info depth 10 score cp 15 time 33 nodes 45304 nps 1372848 pv g1f3
info depth 11 score cp 17 time 70 nodes 110415 nps 1577357 pv d2d4
info depth 12 score cp 17 time 115 nodes 205789 nps 1789469 pv d2d4
info depth 13 score cp 10 time 278 nodes 583508 nps 2098949 pv c2c4
info depth 14 score cp 11 time 333 nodes 711813 nps 2137576 pv c2c4
info depth 15 score cp 20 time 748 nodes 1715012 nps 2292796 pv e2e4
info depth 16 score cp 22 time 874 nodes 2020643 nps 2311948 pv e2e4
info depth 17 score cp 22 time 1063 nodes 2474224 nps 2327586 pv e2e4
info depth 18 score cp 22 time 1267 nodes 2973716 nps 2347052 pv e2e4
info depth 19 score cp 33 time 2039 nodes 4837025 nps 2372253 pv e2e4
info depth 20 score cp 18 upperbound time 3267 nodes 7952978 nps 2434336
info depth 20 score cp 31 time 4298 nodes 10646595 nps 2477104 pv e2e4
info depth 21 score cp 26 time 5042 nodes 12465878 nps 2472407 pv e2e4
info depth 22 score cp 32 time 6370 nodes 15935342 nps 2501623 pv e2e4
bestmove e2e4
15935342 nodes 2501623 nps
```

```py
Elo   | -0.37 +- 2.31 (95%)
SPRT  | 10.0+0.10s Threads=1 Hash=1MB
LLR   | 3.00 (-2.94, 2.94) [-5.00, 0.00]
Games | N: 37444 W: 10147 L: 10187 D: 17110
Penta | [898, 4461, 8046, 4417, 900]
https://gedas.pythonanywhere.com/test/1435/
```

### 5.10

64 bit: 4077 bytes (-10)

MD5: 004cdab043bfc3fe921a069441e3e32e

Nanosecond time management (idea by sqrmax)

```py
info depth 1 score cp 14 time 0 nodes 13 nps 168175 pv e2e3
info depth 2 score cp 17 time 0 nodes 118 nps 202505 pv b1c3
info depth 3 score cp 26 time 0 nodes 263 nps 331067 pv b1c3
info depth 4 score cp 16 time 1 nodes 493 nps 411484 pv b1c3
info depth 5 score cp 10 time 2 nodes 878 nps 435991 pv b1c3
info depth 6 score cp 17 time 3 nodes 1625 nps 480698 pv b1c3
info depth 7 score cp 15 time 11 nodes 4831 nps 410718 pv d2d3
info depth 8 score cp 17 time 23 nodes 7442 nps 315830 pv d2d3
info depth 9 score cp 22 time 55 nodes 21518 nps 387921 pv g1f3
info depth 10 score cp 15 time 91 nodes 45304 nps 492900 pv g1f3
info depth 11 score cp 17 time 180 nodes 110415 nps 612014 pv d2d4
info depth 12 score cp 17 time 344 nodes 205789 nps 598217 pv d2d4
info depth 13 score cp 10 time 834 nodes 583508 nps 699548 pv c2c4
info depth 14 score cp 11 time 992 nodes 711813 nps 717077 pv c2c4
info depth 15 score cp 20 time 2208 nodes 1715012 nps 776382 pv e2e4
info depth 16 score cp 22 time 2583 nodes 2020643 nps 782255 pv e2e4
info depth 17 score cp 22 time 3173 nodes 2474224 nps 779699 pv e2e4
info depth 18 score cp 22 time 3789 nodes 2973716 nps 784788 pv e2e4
info depth 19 score cp 33 time 6153 nodes 4837025 nps 786005 pv e2e4
info depth 20 score cp 18 upperbound time 9827 nodes 7952978 nps 809233
info depth 20 score cp 31 time 12787 nodes 10646595 nps 832589 pv e2e4
info depth 21 score cp 26 time 14832 nodes 12465878 nps 840439 pv e2e4
info depth 22 score cp 32 time 18471 nodes 15935342 nps 862721 pv e2e4
bestmove e2e4
15935342 nodes 12340607 nps
```

```py
Elo   | 8.02 +- 6.37 (95%)
SPRT  | 10.0+0.10s Threads=1 Hash=1MB
LLR   | 2.98 (-2.94, 2.94) [-5.00, 0.00]
Games | N: 4850 W: 1330 L: 1218 D: 2302
Penta | [99, 546, 1052, 600, 128]
https://gedas.pythonanywhere.com/test/1436/
```

### 5.11

64 bit: 4072 bytes (-5)

MD5: cc9035583f8f4d15e472d80db677c3ce

TT length power of 2 (idea by sqrmax)

```py
info depth 1 score cp 14 time 0 nodes 13 nps 343606 pv e2e3
info depth 2 score cp 17 time 0 nodes 118 nps 444885 pv b1c3
info depth 3 score cp 26 time 0 nodes 263 nps 727604 pv b1c3
info depth 4 score cp 16 time 0 nodes 493 nps 922899 pv b1c3
info depth 5 score cp 10 time 0 nodes 878 nps 882971 pv b1c3
info depth 6 score cp 17 time 1 nodes 1625 nps 1030180 pv b1c3
info depth 7 score cp 15 time 4 nodes 4831 nps 1122357 pv d2d3
info depth 8 score cp 17 time 6 nodes 7589 nps 1141420 pv d2d3
info depth 9 score cp 22 time 18 nodes 23492 nps 1269368 pv g1f3
info depth 10 score cp 7 upperbound time 24 nodes 32079 nps 1331330
info depth 10 score cp 7 time 27 nodes 36805 nps 1351231 pv g1f3
info depth 11 score cp 15 time 71 nodes 115161 nps 1605719 pv g1f3
info depth 12 score cp 13 time 112 nodes 200963 nps 1779919 pv g1f3
info depth 13 score cp 11 time 234 nodes 479851 nps 2044216 pv g1f3
info depth 14 score cp 17 time 380 nodes 833084 nps 2189598 pv g1f3
info depth 15 score cp 22 time 625 nodes 1441701 nps 2304502 pv g1f3
info depth 16 score cp 18 time 884 nodes 2059426 nps 2328585 pv g1f3
info depth 17 score cp 29 time 1630 nodes 3874013 nps 2375403 pv e2e4
info depth 18 score cp 29 time 2019 nodes 4812471 nps 2382879 pv e2e4
info depth 19 score cp 29 time 2336 nodes 5581204 nps 2389148 pv e2e4
info depth 20 score cp 22 time 3520 nodes 8633096 nps 2452398 pv e2e4
info depth 21 score cp 18 time 5400 nodes 13413472 nps 2483814 pv e2e4
info depth 22 score cp 22 time 8472 nodes 21230869 nps 2505859 pv e2e4
bestmove e2e4
21230869 nodes 2505847 nps
```

```py
Elo   | 0.89 +- 3.30 (95%)
SPRT  | 10.0+0.10s Threads=1 Hash=1MB
LLR   | 2.98 (-2.94, 2.94) [-5.00, 0.00]
Games | N: 18732 W: 5063 L: 5015 D: 8654
Penta | [443, 2263, 3961, 2201, 498]
https://gedas.pythonanywhere.com/test/1437/
```

### 5.12

64 bit: 4069 bytes (-3)

MD5: 6e9404f7fc1ad8504e2a46dda668cd45

King material UB size save

```py
Elo   | 0.44 +- 2.94 (95%)
SPRT  | 10.0+0.10s Threads=1 Hash=1MB
LLR   | 3.00 (-2.94, 2.94) [-5.00, 0.00]
Games | N: 22844 W: 6166 L: 6137 D: 10541
Penta | [536, 2711, 4905, 2728, 542]
https://gedas.pythonanywhere.com/test/1445/
```

### 5.13

64 bit: 4065 bytes (-4)

MD5: d5aaf2bf63e0d45696925e1ff10e833b

Smaller max_time

```py
Elo   | 0.76 +- 3.22 (95%)
SPRT  | 10.0+0.10s Threads=1 Hash=1MB
LLR   | 2.98 (-2.94, 2.94) [-5.00, 0.00]
Games | N: 19744 W: 5457 L: 5414 D: 8873
Penta | [485, 2371, 4129, 2390, 497]
https://gedas.pythonanywhere.com/test/1446/
```

### 5.14

64 bit: 4081 bytes (+16)

MD5: 17253e37137ed73df8d5caebd40b6999

Phalanx pawn evaluation

```py
info depth 1 score cp 15 lowerbound time 0 nodes 18 nps 442760
info depth 1 score cp 28 time 0 nodes 38 nps 438070 pv b1c3
info depth 2 score cp 17 time 0 nodes 115 nps 438300 pv b1c3
info depth 3 score cp 27 time 0 nodes 235 nps 685776 pv b1c3
info depth 4 score cp 17 time 0 nodes 396 nps 779687 pv b1c3
info depth 5 score cp 3 time 0 nodes 642 nps 866487 pv b1c3
info depth 6 score cp 15 time 1 nodes 1117 nps 1028493 pv b1c3
info depth 7 score cp 15 time 2 nodes 3295 nps 1138565 pv b1c3
info depth 8 score cp 17 time 5 nodes 6115 nps 1150056 pv b1c3
info depth 9 score cp 12 time 12 nodes 15300 nps 1190752 pv g1f3
info depth 10 score cp 23 time 34 nodes 49868 nps 1425896 pv g1f3
info depth 11 score cp 23 time 64 nodes 107838 nps 1674313 pv g1f3
info depth 12 score cp 17 time 109 nodes 213176 nps 1939807 pv g1f3
info depth 13 score cp 23 time 203 nodes 446896 nps 2192076 pv d2d4
info depth 14 score cp 17 time 283 nodes 651064 nps 2293992 pv d2d4
info depth 15 score cp 14 time 467 nodes 1143991 nps 2448646 pv d2d4
info depth 16 score cp 12 time 808 nodes 2064177 nps 2552675 pv d2d4
info depth 17 score cp 15 time 1207 nodes 3117565 nps 2582184 pv d2d4
info depth 18 score cp 30 lowerbound time 1902 nodes 4979082 nps 2616945
info depth 18 score cp 15 time 2302 nodes 6057680 nps 2631146 pv d2d4
info depth 19 score cp 16 time 3400 nodes 8868087 nps 2607960 pv d2d4
info depth 20 score cp 31 lowerbound time 5121 nodes 13184820 nps 2574406
info depth 20 score cp 11 time 6137 nodes 15833184 nps 2579605 pv e2e4
info depth 21 score cp 26 lowerbound time 6370 nodes 16470929 nps 2585628
info depth 21 score cp 26 time 6893 nodes 17852176 nps 2589560 pv e2e4
info depth 22 score cp 26 time 8202 nodes 21293740 nps 2595952 pv e2e4
bestmove e2e4
21293740 nodes 2595938 nps
```

```py
Elo   | 12.08 +- 6.67 (95%)
SPRT  | 10.0+0.10s Threads=1 Hash=1MB
LLR   | 3.04 (-2.94, 2.94) [0.00, 5.00]
Games | N: 4776 W: 1398 L: 1232 D: 2146
Penta | [107, 538, 976, 616, 151]
https://gedas.pythonanywhere.com/test/1454/
```

```py
Elo   | 17.86 +- 7.98 (95%)
SPRT  | 60.0+0.60s Threads=1 Hash=1MB
LLR   | 2.95 (-2.94, 2.94) [0.00, 5.00]
Games | N: 2628 W: 717 L: 582 D: 1329
Penta | [29, 272, 598, 365, 50]
https://gedas.pythonanywhere.com/test/1456/
```

### 5.15

64 bit: 4079 bytes (-2)

MD5: 7e57991d4f56592a69fff2b7fd13dd64

Phase UB size save

### 5.16

64 bit: 4078 bytes (-1)

MD5: 3ca0af41be71884df02512a145ceed55

Reverse loop size save (idea by sqrmax)

### 5.17

64 bit: 4074 bytes (-4)

MD5: b65f60256f3f7e884bf0ed036eb4da42

Decrease loader size
* Push 1 pop X is 1 byte smaller than xor X, X + inc X
* Replacing rdi and rax to edi and eax because the addresses are all within 32 bit range

### 5.18

64 bit: 4072 bytes (-2)

MD5: 0c87778b8544a8e245dedd2a453332a0

Decrease loader size
* Put payload_compressed in .text section so we can compute offset
* Use `lea ebp, [rsi + getbit - payload_compressed]` instead of `mov ebp, getbit`

### 5.18

64 bit: 4070 bytes (-2)

MD5: d8c43ff83602ee63ec6b60bacff615fb

Decrease loader size

### 5.19

64 bit: 4086 bytes (+16)

MD5: a89f5646c9898b63c29ec618e027cac3

Move score pruning

```py
info depth 1 score cp 15 lowerbound time 0 nodes 18 nps 416204
info depth 1 score cp 28 time 0 nodes 38 nps 423525 pv b1c3
info depth 2 score cp 17 time 0 nodes 115 nps 439962 pv b1c3
info depth 3 score cp 27 time 0 nodes 235 nps 685249 pv b1c3
info depth 4 score cp 17 time 0 nodes 396 nps 806080 pv b1c3
info depth 5 score cp 3 time 0 nodes 642 nps 889197 pv b1c3
info depth 6 score cp 15 time 1 nodes 1117 nps 1016960 pv b1c3
info depth 7 score cp 15 time 2 nodes 3295 nps 1107543 pv b1c3
info depth 8 score cp 17 time 5 nodes 6115 nps 1091720 pv b1c3
info depth 9 score cp 12 time 14 nodes 15300 nps 1060604 pv g1f3
info depth 10 score cp 23 time 43 nodes 49864 nps 1146095 pv g1f3
info depth 11 score cp 23 time 82 nodes 107831 nps 1303563 pv g1f3
info depth 12 score cp 17 time 140 nodes 213117 nps 1521394 pv g1f3
info depth 13 score cp 23 time 298 nodes 512363 nps 1719287 pv d2d4
info depth 14 score cp 17 time 395 nodes 709018 nps 1793100 pv d2d4
info depth 15 score cp 10 time 580 nodes 1146961 nps 1974465 pv d2d4
info depth 16 score cp 10 time 948 nodes 2060455 nps 2172002 pv d2d4
info depth 17 score cp 11 time 1511 nodes 3262249 nps 2157617 pv d2d4
info depth 18 score cp 15 time 2267 nodes 5048581 nps 2226952 pv d2d4
info depth 19 score cp 17 time 3059 nodes 6943926 nps 2269839 pv d2d4
info depth 20 score cp 19 time 4255 nodes 9804664 nps 2303794 pv d2d4
info depth 21 score cp 20 time 5762 nodes 13356974 nps 2317983 pv d2d4
info depth 22 score cp 20 time 7029 nodes 16388538 nps 2331380 pv d2d4
bestmove d2d4
16388538 nodes 2331365 nps
```

```py
Elo   | 14.19 +- 7.39 (95%)
SPRT  | 10.0+0.10s Threads=1 Hash=1MB
LLR   | 2.97 (-2.94, 2.94) [0.00, 5.00]
Games | N: 3846 W: 1108 L: 951 D: 1787
Penta | [79, 422, 820, 467, 135]
https://gedas.pythonanywhere.com/test/1493/
```

```py
Elo   | 14.06 +- 6.96 (95%)
SPRT  | 60.0+0.60s Threads=1 Hash=1MB
LLR   | 2.95 (-2.94, 2.94) [0.00, 5.00]
Games | N: 3362 W: 890 L: 754 D: 1718
Penta | [40, 341, 806, 431, 63]
https://gedas.pythonanywhere.com/test/1494/
```

### 5.20

64 bit: 4085 bytes (-1)

MD5: 972d77dd7665b3226ae8ff821dfec939

Decrease loader size

### 5.21

64 bit: 4052 bytes (-33)

MD5: 1682485d0e416f663d31e5834facc092

Loader ELF header smashing

### 5.22

64 bit: 4044 bytes (-8)

MD5: 72c9a9e8693ac7e87f170b318054b2b3

ELF / PHDR header overlapping

### 5.23

64 bit: 3994 bytes (-50)

MD5: cc77f5736fec1ac402cefb653e4bbe12

Disable second instruction scheduling pass

```py
Elo   | -1.50 +- 1.93 (95%)
SPRT  | 10.0+0.10s Threads=1 Hash=1MB
LLR   | 0.50 (-2.94, 2.94) [-5.00, 0.00]
Games | N: 53778 W: 14613 L: 14845 D: 24320
Penta | [1375, 6378, 11515, 6346, 1275]
https://gedas.pythonanywhere.com/test/1545/
```

### 5.24

64 bit: 3994 bytes (=)

MD5: 256dfb76b9d045d7cafae3fe754254c6

Move score pruning margin 128

```py
info depth 1 score cp 15 lowerbound time 0 nodes 18 nps 444279
info depth 1 score cp 28 time 0 nodes 38 nps 419889 pv b1c3
info depth 2 score cp 17 time 0 nodes 115 nps 443998 pv b1c3
info depth 3 score cp 27 time 0 nodes 235 nps 642953 pv b1c3
info depth 4 score cp 17 time 0 nodes 396 nps 703552 pv b1c3
info depth 5 score cp 3 time 0 nodes 642 nps 739147 pv b1c3
info depth 6 score cp 15 time 1 nodes 1117 nps 903745 pv b1c3
info depth 7 score cp 15 time 3 nodes 3295 nps 1065184 pv b1c3
info depth 8 score cp 17 time 5 nodes 6115 nps 1058756 pv b1c3
info depth 9 score cp 12 time 14 nodes 15300 nps 1088555 pv g1f3
info depth 10 score cp 23 time 38 nodes 49868 nps 1290026 pv g1f3
info depth 11 score cp 23 time 70 nodes 107837 nps 1526241 pv g1f3
info depth 12 score cp 17 time 118 nodes 212456 nps 1792455 pv g1f3
info depth 13 score cp 11 time 210 nodes 427911 nps 2034624 pv g1f3
info depth 14 score cp 14 time 374 nodes 812329 nps 2169036 pv b1c3
info depth 15 score cp 21 time 627 nodes 1372980 nps 2186862 pv d2d4
info depth 16 score cp 25 time 1058 nodes 2304583 nps 2176520 pv e2e4
info depth 17 score cp 24 time 1189 nodes 2603632 nps 2188566 pv e2e4
info depth 18 score cp 24 time 1549 nodes 3438587 nps 2219711 pv e2e4
info depth 19 score cp 24 time 1865 nodes 4144751 nps 2222328 pv e2e4
info depth 20 score cp 17 time 3186 nodes 7114318 nps 2232459 pv e2e4
info depth 21 score cp 21 time 4034 nodes 8968036 nps 2222809 pv e2e4
info depth 22 score cp 15 time 8672 nodes 19165661 nps 2209888 pv g1f3
bestmove g1f3
19165661 nodes 2209875 nps
```

```py
Elo   | 8.59 +- 5.43 (95%)
SPRT  | 10.0+0.10s Threads=1 Hash=1MB
LLR   | 3.01 (-2.94, 2.94) [0.00, 5.00]
Games | N: 6998 W: 2008 L: 1835 D: 3155
Penta | [156, 817, 1411, 928, 187]
https://gedas.pythonanywhere.com/test/1553/
```

```
Elo   | 16.93 +- 7.96 (95%)
SPRT  | 60.0+0.60s Threads=1 Hash=1MB
LLR   | 2.96 (-2.94, 2.94) [0.00, 5.00]
Games | N: 2978 W: 836 L: 691 D: 1451
Penta | [45, 325, 634, 410, 75]
https://gedas.pythonanywhere.com/test/1555/
```

### 5.25

64 bit: 3988 bytes (-6)

MD5: 0db203b3223fa8c4ac9ae14a8394f685

Unify eval value calculation

### 5.26

64 bit: 3982 bytes (-6)

MD5: e6b9bc5c42f72a07b92db745d42e4e27

Smaller atoi and go

### 5.27

64 bit: 3974 bytes (-8)

MD5: 0a85a595eefa67a864c1f8b4f600475a

Smaller castling

### 5.28

64 bit: 3973 bytes (-1)

MD5: bf43dd4006235e8074f90dfe999daaa4

Smaller diagonal mask compute

### 5.29

64 bit: 3964 bytes (-9)

MD5: 839140c325485da2e64d130c9f18413a

Inline comparisons

### 5.30

64 bit: 3961 bytes (-3)

MD5: 10192d454debe922fc4c6c5ac59c9c26

Reduce size of king and knight

Test vs 5.25:
```py
Elo   | 2.23 +- 4.07 (95%)
SPRT  | 10.0+0.10s Threads=1 Hash=1MB
LLR   | 3.07 (-2.94, 2.94) [-5.00, 0.00]
Games | N: 12760 W: 3604 L: 3522 D: 5634
Penta | [316, 1551, 2617, 1527, 369]
https://gedas.pythonanywhere.com/test/1572/
```

### 5.31

64 bit: 3958 bytes (-3)

MD5: 2b04563fd2726e03164a5837c3fa0933

Smaller getl

### 5.32

64 bit: 3960 bytes (+2)

MD5: 7c47631f5e6dd3d71f2e7581858eb435

Repetition detection fix

Previously it wouldn't detect repetition to startpos or the first move after capture

```py
Elo   | 0.83 +- 3.24 (95%)
SPRT  | 10.0+0.10s Threads=1 Hash=1MB
LLR   | 3.00 (-2.94, 2.94) [-5.00, 0.00]
Games | N: 19344 W: 5390 L: 5344 D: 8610
Penta | [486, 2269, 4107, 2333, 477]
https://gedas.pythonanywhere.com/test/1581/
```

### 5.33

64 bit: 3961 bytes (+1)

MD5: d70c18ba5fa1aaa4604ae04b870b3958

Moves not in stack

Even though no gains and one byte larger, it saves stack size and removes the need
for special handling for Windows builds

```py
Elo   | -0.50 +- 2.21 (95%)
SPRT  | 10.0+0.10s Threads=1 Hash=1MB
LLR   | 2.99 (-2.94, 2.94) [-5.00, 0.00]
Games | N: 41302 W: 11344 L: 11403 D: 18555
Penta | [1052, 4848, 8853, 4903, 995]
https://gedas.pythonanywhere.com/test/1587/
```

### 5.34

4 threads: 4079 bytes (+118)
MD5: cf9ca8af00440fa1b18059a1c9ac72b9

1 thread: 4009 bytes (-48)
MD5: 1f4bef9c77a532ce0be81900390df1f2

* Lazy SMP
* Remove full repetition detection

```py
info depth 1 score cp 15 lowerbound time 0 nodes 18 nps 429215
info depth 1 score cp 28 time 0 nodes 38 nps 405428 pv b1c3
info depth 2 score cp 17 time 0 nodes 115 nps 435714 pv b1c3
info depth 3 score cp 27 time 0 nodes 235 nps 677946 pv b1c3
info depth 4 score cp 17 time 0 nodes 396 nps 798333 pv b1c3
info depth 5 score cp 3 time 0 nodes 642 nps 888285 pv b1c3
info depth 6 score cp 15 time 1 nodes 1117 nps 1023623 pv b1c3
info depth 7 score cp 15 time 2 nodes 3295 nps 1128595 pv b1c3
info depth 8 score cp 17 time 5 nodes 6115 nps 1140574 pv b1c3
info depth 9 score cp 12 time 13 nodes 15300 nps 1162317 pv g1f3
info depth 10 score cp 23 time 36 nodes 49868 nps 1384535 pv g1f3
info depth 11 score cp 23 time 67 nodes 107837 nps 1600450 pv g1f3
info depth 12 score cp 17 time 115 nodes 212456 nps 1838186 pv g1f3
info depth 13 score cp 11 time 211 nodes 427911 nps 2024799 pv g1f3
info depth 14 score cp 14 time 374 nodes 812329 nps 2171432 pv b1c3
info depth 15 score cp 21 time 611 nodes 1372980 nps 2244626 pv d2d4
info depth 16 score cp 25 time 1012 nodes 2304583 nps 2276928 pv e2e4
info depth 17 score cp 24 time 1149 nodes 2603632 nps 2264675 pv e2e4
info depth 18 score cp 24 time 1509 nodes 3438587 nps 2278399 pv e2e4
info depth 19 score cp 24 time 1816 nodes 4144751 nps 2281440 pv e2e4
info depth 20 score cp 17 time 3113 nodes 7114318 nps 2284987 pv e2e4
info depth 21 score cp 21 time 3932 nodes 8968036 nps 2280389 pv e2e4
info depth 22 score cp 15 time 8515 nodes 19165661 nps 2250579 pv g1f3
19165661 nodes 2250567 nps
```

1 thread vs 5.33:
```py
Elo   | 2.31 +- 15.72 (95%)
SPRT  | 10.0+0.10s Threads=1 Hash=1MB
LLR   | 0.19 (-2.94, 2.94) [-5.00, 0.00]
Games | N: 752 W: 207 L: 202 D: 343
Penta | [18, 79, 173, 92, 14]
https://gedas.pythonanywhere.com/test/1602/
```

2v1 10+0.1
```py
Score of 4kc-2t vs 4kc-main: 181 - 59 - 180  [0.645] 420
...      4kc-2t playing White: 133 - 8 - 69  [0.798] 210
...      4kc-2t playing Black: 48 - 51 - 111  [0.493] 210
...      White vs Black: 184 - 56 - 180  [0.652] 420
Elo difference: 103.9 +/- 25.3, LOS: 100.0 %, DrawRatio: 42.9 %
```

4v1 10+0.1
```py
Score of 4kc-4t vs 4kc-main: 118 - 14 - 68  [0.760] 200
...      4kc-4t playing White: 76 - 2 - 22  [0.870] 100
...      4kc-4t playing Black: 42 - 12 - 46  [0.650] 100
...      White vs Black: 88 - 44 - 68  [0.610] 200
Elo difference: 200.2 +/- 41.5, LOS: 100.0 %, DrawRatio: 34.0 %
```

8v1 10+0.1
```py
Score of 4kc-8t vs 4kc-main: 135 - 7 - 58  [0.820] 200
...      4kc-8t playing White: 84 - 0 - 16  [0.920] 100
...      4kc-8t playing Black: 51 - 7 - 42  [0.720] 100
...      White vs Black: 91 - 51 - 58  [0.600] 200
Elo difference: 263.4 +/- 45.2, LOS: 100.0 %, DrawRatio: 29.0 %
```

8moves book 4v1 10+0.1:
```py
Score of 4kc-4t vs 4kc-main: 166 - 16 - 118  [0.750] 300
...      4kc-4t playing White: 84 - 8 - 58  [0.753] 150
...      4kc-4t playing Black: 82 - 8 - 60  [0.747] 150
...      White vs Black: 92 - 90 - 118  [0.503] 300
Elo difference: 190.8 +/- 31.4, LOS: 100.0 %, DrawRatio: 39.3 %
```

8moves book 8v1 10+0.1:
```py
Score of 4kc-8t vs 4kc-main: 197 - 6 - 97  [0.818] 300
...      4kc-8t playing White: 97 - 3 - 50  [0.813] 150
...      4kc-8t playing Black: 100 - 3 - 47  [0.823] 150
...      White vs Black: 100 - 103 - 97  [0.495] 300
Elo difference: 261.5 +/- 34.6, LOS: 100.0 %, DrawRatio: 32.3 %
```

### 6.0

Equivalent to 5.34

First version with Lazy SMP

4 threads: 4079 bytes
MD5: cf9ca8af00440fa1b18059a1c9ac72b9

1 thread: 4009 bytes 
MD5: 1f4bef9c77a532ce0be81900390df1f2

8moves book 10+0.1:
```py
Score of 4kc-1t vs 4ku-3.0: 499 - 331 - 673  [0.556] 1503
...      4kc-1t playing White: 261 - 146 - 344  [0.577] 751
...      4kc-1t playing Black: 238 - 185 - 329  [0.535] 752
...      White vs Black: 446 - 384 - 673  [0.521] 1503
Elo difference: 39.0 +/- 13.1, LOS: 100.0 %, DrawRatio: 44.8 %
```

8moves book 10+0.1:
```py
Score of 4kc-1t vs 4ku-3.1: 4333 - 3631 - 5036  [0.527] 13000
...      4kc-1t playing White: 2248 - 1698 - 2554  [0.542] 6500
...      4kc-1t playing Black: 2085 - 1933 - 2482  [0.512] 6500
...      White vs Black: 4181 - 3783 - 5036  [0.515] 13000
Elo difference: 18.8 +/- 4.7, LOS: 100.0 %, DrawRatio: 38.7 %
```

### 6.1

1MB child thread stack size

4 threads: 4074 bytes (-5)
MD5: 02b0488819271dd921a0bd807bcfd539

1 thread: 4008 bytes (-1)
MD5: 0b875d4c3310332f4d3bacb53f1f610c

```py
info depth 1 score cp 15 lowerbound time 0 nodes 18 nps 387763
info depth 1 score cp 28 time 0 nodes 38 nps 401500 pv b1c3
info depth 2 score cp 17 time 0 nodes 115 nps 437355 pv b1c3
info depth 3 score cp 27 time 0 nodes 235 nps 679906 pv b1c3
info depth 4 score cp 17 time 0 nodes 396 nps 783578 pv b1c3
info depth 5 score cp 3 time 0 nodes 642 nps 856771 pv b1c3
info depth 6 score cp 15 time 1 nodes 1117 nps 977312 pv b1c3
info depth 7 score cp 15 time 2 nodes 3295 nps 1128061 pv b1c3
info depth 8 score cp 17 time 5 nodes 6115 nps 1109182 pv b1c3
info depth 9 score cp 12 time 13 nodes 15300 nps 1156618 pv g1f3
info depth 10 score cp 23 time 38 nodes 49868 nps 1283838 pv g1f3
info depth 11 score cp 23 time 71 nodes 107837 nps 1512277 pv g1f3
info depth 12 score cp 17 time 119 nodes 212456 nps 1773389 pv g1f3
info depth 13 score cp 11 time 209 nodes 427911 nps 2041965 pv g1f3
info depth 14 score cp 14 time 367 nodes 812329 nps 2210419 pv b1c3
info depth 15 score cp 21 time 600 nodes 1372980 nps 2286170 pv d2d4
info depth 16 score cp 25 time 992 nodes 2304583 nps 2322119 pv e2e4
info depth 17 score cp 24 time 1118 nodes 2603632 nps 2327508 pv e2e4
info depth 18 score cp 24 time 1469 nodes 3438587 nps 2339953 pv e2e4
info depth 19 score cp 24 time 1775 nodes 4144751 nps 2334123 pv e2e4
info depth 20 score cp 17 time 3064 nodes 7114318 nps 2321822 pv e2e4
info depth 21 score cp 21 time 3877 nodes 8968036 nps 2313001 pv e2e4
info depth 22 score cp 15 time 8355 nodes 19165661 nps 2293837 pv g1f3
19165661 nodes 2293826 nps
```

10+0.1:
```py
Score of 4kc-1MB vs 4kc-8MB: 1214 - 1174 - 3112  [0.504] 5500
...      4kc-1MB playing White: 663 - 510 - 1578  [0.528] 2751
...      4kc-1MB playing Black: 551 - 664 - 1534  [0.479] 2749
...      White vs Black: 1327 - 1061 - 3112  [0.524] 5500
Elo difference: 2.5 +/- 6.0, LOS: 79.3 %, DrawRatio: 56.6 %
```

### 6.2

Pass thread data to search

4 threads: 4072 bytes (-2)
MD5: 5ab5dfda260319d0ee43b85e453a62a4

1 thread: 3971 bytes (-37)
MD5: 43b6b63a71d3d7d93457c9225d10b5d2

```py
Elo   | 3.12 +- 4.54 (95%)
SPRT  | 10.0+0.10s Threads=1 Hash=1MB
LLR   | 2.95 (-2.94, 2.94) [-5.00, 0.00]
Games | N: 9916 W: 2727 L: 2638 D: 4551
Penta | [228, 1205, 2029, 1242, 254]
https://gedas.pythonanywhere.com/test/1610/
```

### 6.3

Exclude unneeded variables for mini

4 threads: 4070 bytes (-2)
MD5: f25799cc4e6cc428d40cf5962587ccac

1 thread: 3973 bytes (+2)
MD5: 53583adca54039bde41b8f550690f31e

### 6.4

Optimize mobility

4 threads: 4072 bytes (+2)
MD5: b1eb38153c20cbba0a180cbc40c3d5ee

1 thread: 3975 bytes (+2)
MD5: 2e1dfc13f72cbc23bb53a5f621b620f3

```py
Elo   | 9.26 +- 6.80 (95%)
SPRT  | 10.0+0.10s Threads=1 Hash=1MB
LLR   | 2.97 (-2.94, 2.94) [-5.00, 0.00]
Games | N: 4316 W: 1235 L: 1120 D: 1961
Penta | [84, 507, 889, 566, 112]
https://gedas.pythonanywhere.com/test/1612/
```

### 6.5

Tune search parameters

4 threads: 4077 bytes (+5)
MD5: caa08cf41deecc29e034de8229053240

1 thread: 3979 bytes (+4)
MD5: f203985b4bb719662ccdf15e97b28a84

```py
info depth 1 score cp 15 lowerbound time 0 nodes 18 nps 332042
info depth 1 score cp 28 time 0 nodes 38 nps 333073 pv b1c3
info depth 2 score cp 17 time 0 nodes 116 nps 335912 pv b1c3
info depth 3 score cp 27 time 0 nodes 236 nps 550081 pv b1c3
info depth 4 score cp 17 time 0 nodes 397 nps 657071 pv b1c3
info depth 5 score cp 3 time 0 nodes 638 nps 742823 pv b1c3
info depth 6 score cp 15 time 1 nodes 1122 nps 895592 pv b1c3
info depth 7 score cp 15 time 3 nodes 3220 nps 1034810 pv b1c3
info depth 8 score cp 17 time 5 nodes 5972 nps 1032511 pv b1c3
info depth 9 score cp 18 time 17 nodes 18503 nps 1067674 pv g1f3
info depth 10 score cp 17 time 57 nodes 75610 nps 1316583 pv b1c3
info depth 11 score cp 16 time 103 nodes 154136 nps 1491847 pv b1c3
info depth 12 score cp 17 time 153 nodes 257349 nps 1680836 pv b1c3
info depth 13 score cp 7 time 258 nodes 493457 nps 1905860 pv b1c3
info depth 14 score cp 22 lowerbound time 410 nodes 832055 nps 2025597
info depth 14 score cp 27 time 451 nodes 923350 nps 2045019 pv e2e4
info depth 15 score cp 27 time 526 nodes 1084739 nps 2061565 pv e2e4
info depth 16 score cp 27 time 723 nodes 1513982 nps 2092821 pv e2e4
info depth 17 score cp 25 time 912 nodes 1936726 nps 2122118 pv e2e4
info depth 18 score cp 25 time 1140 nodes 2449959 nps 2148558 pv e2e4
info depth 19 score cp 31 time 1450 nodes 3136866 nps 2162012 pv e2e4
info depth 20 score cp 16 upperbound time 2255 nodes 4906335 nps 2175155
info depth 20 score cp 15 time 3241 nodes 7088793 nps 2186605 pv e2e4
info depth 21 score cp 25 time 3733 nodes 8157837 nps 2184913 pv e2e4
info depth 22 score cp 40 lowerbound time 4778 nodes 10455647 nps 2188088
info depth 22 score cp 27 time 5580 nodes 12188254 nps 2184273 pv e2e4
12188254 nodes 2184255 nps
```

```py
Elo   | -0.30 +- 2.95 (95%)
SPRT  | 10.0+0.10s Threads=1 Hash=1MB
LLR   | -2.95 (-2.94, 2.94) [0.00, 5.00]
Games | N: 24128 W: 6701 L: 6722 D: 10705
Penta | [660, 2904, 4901, 2995, 604]
https://gedas.pythonanywhere.com/test/1614/
```

```py
Elo   | 10.21 +- 5.86 (95%)
SPRT  | 60.0+0.60s Threads=1 Hash=1MB
LLR   | 2.95 (-2.94, 2.94) [0.00, 5.00]
Games | N: 4972 W: 1321 L: 1175 D: 2476
Penta | [67, 550, 1130, 648, 91]
https://gedas.pythonanywhere.com/test/1613/
```

### 6.6

Extact bbs in eval 

4 threads: 4077 bytes (=)
MD5: 430b6de8886f4bc3c1a417efa3aeb6b9

1 thread: 3978 bytes (-1)
MD5: ed595ef87eff6385b66dcf4536ff5dfc

### 6.7

Simplify king atack eval

4 threads: 4068 bytes (-9)
MD5: d0f25db83cc2c9dc5134be999ecd8b41

1 thread: 3968 bytes (-10)
MD5: 01d694a08a0b72bb82a42a3b7b74bf42

```py
info depth 1 score cp 15 lowerbound time 0 nodes 18 nps 341646
info depth 1 score cp 28 time 0 nodes 38 nps 369394 pv b1c3
info depth 2 score cp 17 time 0 nodes 116 nps 407400 pv b1c3
info depth 3 score cp 27 time 0 nodes 236 nps 643339 pv b1c3
info depth 4 score cp 17 time 0 nodes 417 nps 764538 pv b1c3
info depth 5 score cp 4 time 0 nodes 683 nps 839673 pv b1c3
info depth 6 score cp 16 time 1 nodes 1165 nps 958672 pv b1c3
info depth 7 score cp 12 time 3 nodes 3649 nps 1050496 pv b1c3
info depth 8 score cp 25 time 8 nodes 9200 nps 1097951 pv d2d3
info depth 9 score cp 21 time 15 nodes 18053 nps 1190460 pv d2d3
info depth 10 score cp 7 time 21 nodes 26694 nps 1238394 pv d2d3
info depth 11 score cp 22 lowerbound time 42 nodes 58076 nps 1361281
info depth 11 score cp 21 time 77 nodes 124126 nps 1592214 pv g1f3
info depth 12 score cp 17 time 139 nodes 256651 nps 1837338 pv g1f3
info depth 13 score cp 9 time 238 nodes 478324 nps 2007641 pv b1c3
info depth 14 score cp 17 time 342 nodes 712003 nps 2077007 pv b1c3
info depth 15 score cp 28 time 768 nodes 1682237 nps 2189013 pv e2e4
info depth 16 score cp 13 upperbound time 954 nodes 2095504 nps 2195908
info depth 16 score cp 17 time 1022 nodes 2245475 nps 2196477 pv e2e4
info depth 17 score cp 11 time 1301 nodes 2859629 nps 2197510 pv e2e4
info depth 18 score cp 12 time 2147 nodes 4719701 nps 2197621 pv e2e4
info depth 19 score cp 15 time 3524 nodes 7711467 nps 2187687 pv d2d4
info depth 20 score cp 16 time 4247 nodes 9277351 nps 2184261 pv d2d4
info depth 21 score cp 31 lowerbound time 6567 nodes 14318416 nps 2180070
info depth 21 score cp 32 time 7230 nodes 15789710 nps 2183857 pv e2e4
info depth 22 score cp 17 upperbound time 8313 nodes 17984880 nps 2163439
info depth 22 score cp 28 time 8683 nodes 18727994 nps 2156809 pv e2e4
18727994 nodes 2156798 nps
```

```py
Elo   | 2.04 +- 3.98 (95%)
SPRT  | 10.0+0.10s Threads=1 Hash=1MB
LLR   | 3.01 (-2.94, 2.94) [-5.00, 0.00]
Games | N: 13084 W: 3625 L: 3548 D: 5911
Penta | [334, 1549, 2694, 1636, 329]
https://gedas.pythonanywhere.com/test/1624/
```

### 6.8

LMR move score divisor 384

4 threads: 4055 bytes (-13)
MD5: c9637476761e3d26a0452642c6e36706

1 thread: 3971 bytes (+3)
MD5: 9e427e388ee1c9f5725fabb5158bd991

```py
info depth 1 score cp 15 lowerbound time 0 nodes 18 nps 358794
info depth 1 score cp 28 time 0 nodes 38 nps 370905 pv b1c3
info depth 2 score cp 17 time 0 nodes 116 nps 438371 pv b1c3
info depth 3 score cp 27 time 0 nodes 236 nps 669034 pv b1c3
info depth 4 score cp 17 time 0 nodes 417 nps 787681 pv b1c3
info depth 5 score cp 4 time 0 nodes 683 nps 889476 pv b1c3
info depth 6 score cp 16 time 1 nodes 1165 nps 1034918 pv b1c3
info depth 7 score cp 12 time 3 nodes 3649 nps 1176080 pv b1c3
info depth 8 score cp 25 time 7 nodes 9200 nps 1219443 pv d2d3
info depth 9 score cp 21 time 13 nodes 18053 nps 1310675 pv d2d3
info depth 10 score cp 7 time 19 nodes 26694 nps 1354041 pv d2d3
info depth 11 score cp 22 lowerbound time 39 nodes 58076 nps 1479144
info depth 11 score cp 21 time 73 nodes 124126 nps 1693366 pv g1f3
info depth 12 score cp 17 time 131 nodes 256651 nps 1949529 pv g1f3
info depth 13 score cp 9 time 225 nodes 478324 nps 2122229 pv b1c3
info depth 14 score cp 24 lowerbound time 378 nodes 842665 nps 2226430
info depth 14 score cp 15 time 470 nodes 1054469 nps 2242417 pv d2d4
info depth 15 score cp 20 time 652 nodes 1489472 nps 2284084 pv e2e4
info depth 16 score cp 24 time 804 nodes 1845767 nps 2294174 pv e2e4
info depth 17 score cp 21 time 1157 nodes 2697387 nps 2331297 pv e2e4
info depth 18 score cp 21 time 1464 nodes 3408516 nps 2327815 pv e2e4
info depth 19 score cp 34 time 2269 nodes 5332645 nps 2349834 pv e2e4
info depth 20 score cp 34 time 2731 nodes 6406547 nps 2345107 pv e2e4
info depth 21 score cp 30 time 3618 nodes 8468914 nps 2340665 pv e2e4
info depth 22 score cp 33 time 4532 nodes 10593619 nps 2337472 pv e2e4
10593619 nodes 2337450 nps
```

```py
Elo   | -0.84 +- 3.29 (95%)
SPRT  | 10.0+0.10s Threads=1 Hash=1MB
LLR   | -2.96 (-2.94, 2.94) [0.00, 5.00]
Games | N: 19034 W: 5180 L: 5226 D: 8628
Penta | [461, 2373, 3928, 2261, 494]
https://gedas.pythonanywhere.com/test/1628/
```

```py
Elo   | 4.00 +- 3.13 (95%)
SPRT  | 60.0+0.60s Threads=1 Hash=1MB
LLR   | 2.96 (-2.94, 2.94) [0.00, 5.00]
Games | N: 17820 W: 4711 L: 4506 D: 8603
Penta | [298, 2049, 4029, 2218, 316]
https://gedas.pythonanywhere.com/test/1629/
```

### 6.9

Full open file evaluation

4 threads: 4096 bytes (+41)
MD5: 34712b000e55887f403f7ef3e877a365

1 thread: 4000 bytes (+29)
MD5: 8d293725b87c740b48718e0bf3dec391

```py
info depth 1 score cp 15 lowerbound time 0 nodes 18 nps 407488
info depth 1 score cp 31 time 0 nodes 38 nps 403354 pv b1c3
info depth 2 score cp 17 time 0 nodes 114 nps 443043 pv b1c3
info depth 3 score cp 27 time 0 nodes 234 nps 691740 pv b1c3
info depth 4 score cp 17 time 0 nodes 382 nps 790574 pv b1c3
info depth 5 score cp 3 time 0 nodes 583 nps 882942 pv b1c3
info depth 6 score cp 15 time 0 nodes 903 nps 950438 pv b1c3
info depth 7 score cp 15 time 2 nodes 2428 nps 1061819 pv b1c3
info depth 8 score cp 17 time 3 nodes 3961 nps 1098845 pv b1c3
info depth 9 score cp 15 time 11 nodes 13039 nps 1100641 pv g1f3
info depth 10 score cp 24 time 32 nodes 38208 nps 1188500 pv g1f3
info depth 11 score cp 12 time 58 nodes 79884 nps 1356769 pv g1f3
info depth 12 score cp 19 time 118 nodes 196304 nps 1662029 pv g1f3
info depth 13 score cp 16 time 232 nodes 451148 nps 1938460 pv b1c3
info depth 14 score cp 20 time 326 nodes 664882 nps 2036718 pv b1c3
info depth 15 score cp 17 time 478 nodes 1005483 nps 2101453 pv b1c3
info depth 16 score cp 18 time 693 nodes 1483229 nps 2138130 pv b1c3
info depth 17 score cp 20 time 1185 nodes 2558691 nps 2158288 pv e2e4
info depth 18 score cp 20 time 1493 nodes 3211125 nps 2149939 pv e2e4
info depth 19 score cp 25 time 1918 nodes 4131446 nps 2153543 pv e2e4
info depth 20 score cp 18 time 5503 nodes 11796308 nps 2143542 pv d2d4
info depth 21 score cp 20 time 7028 nodes 15039918 nps 2139897 pv d2d4
info depth 22 score cp 16 time 10948 nodes 23188516 nps 2117934 pv d2d4
23188516 nodes 2117925 nps
```

```py
Elo   | 11.84 +- 6.53 (95%)
SPRT  | 10.0+0.10s Threads=1 Hash=1MB
LLR   | 3.04 (-2.94, 2.94) [0.00, 5.00]
Games | N: 4784 W: 1403 L: 1240 D: 2141
Penta | [97, 543, 989, 626, 137]
https://gedas.pythonanywhere.com/test/1662/
```

Older test with same changes:
```py
Elo   | 9.61 +- 5.91 (95%)
SPRT  | 10.0+0.10s Threads=1 Hash=1MB
LLR   | 2.96 (-2.94, 2.94) [0.00, 5.00]
Games | N: 6146 W: 1810 L: 1640 D: 2696
Penta | [149, 701, 1245, 787, 191]
https://gedas.pythonanywhere.com/test/1473/
```

### 6.10

Split thread head and data

4 threads: 4089 bytes (-5)
MD5: 4d964f093b53b297d0f7df81c379ff7c

1 thread: 3994 bytes (-6)
MD5: 81aff44d7dae1c6428ccaab61290ff24

```py
info depth 1 score cp 15 lowerbound time 0 nodes 18 nps 234375
info depth 1 score cp 31 time 0 nodes 38 nps 79494 pv b1c3
info depth 2 score cp 17 time 1 nodes 114 nps 97008 pv b1c3
info depth 3 score cp 27 time 1 nodes 234 nps 157982 pv b1c3
info depth 4 score cp 17 time 1 nodes 382 nps 207761 pv b1c3
info depth 5 score cp 3 time 2 nodes 583 nps 267489 pv b1c3
info depth 6 score cp 15 time 2 nodes 903 nps 345364 pv b1c3
info depth 7 score cp 15 time 4 nodes 2417 nps 529722 pv b1c3
info depth 8 score cp 17 time 6 nodes 3515 nps 553995 pv b1c3
info depth 9 score cp 14 time 18 nodes 12210 nps 647596 pv b1c3
info depth 10 score cp 16 time 37 nodes 27941 nps 735601 pv g1f3
info depth 11 score cp 26 time 100 nodes 85748 nps 855930 pv g1f3
info depth 12 score cp 16 time 147 nodes 135909 nps 920738 pv g1f3
info depth 13 score cp 14 time 298 nodes 326600 nps 1095043 pv g1f3
info depth 14 score cp 11 time 595 nodes 705870 nps 1185729 pv g1f3
info depth 15 score cp 15 time 810 nodes 968393 nps 1194376 pv g1f3
info depth 16 score cp 30 lowerbound time 1478 nodes 1812291 nps 1225952
info depth 16 score cp 22 time 1558 nodes 1908953 nps 1224732 pv e2e4
info depth 17 score cp 30 time 1907 nodes 2341015 nps 1227197 pv e2e4
info depth 18 score cp 31 time 2333 nodes 2924298 nps 1253207 pv e2e4
info depth 19 score cp 26 time 3096 nodes 3951790 nps 1276045 pv e2e4
info depth 20 score cp 22 time 4910 nodes 6366095 nps 1296341 pv e2e4
info depth 21 score cp 14 time 7558 nodes 9820618 nps 1299285 pv e2e4
info depth 22 score cp 29 lowerbound time 8223 nodes 10674930 nps 1298031
info depth 22 score cp 35 time 11092 nodes 14398609 nps 1298092 pv e2e4
14398609 nodes 1298081 nps
```

```py
Elo   | 2.21 +- 16.74 (95%)
SPRT  | 10.0+0.10s Threads=1 Hash=1MB
LLR   | 0.16 (-2.94, 2.94) [-5.00, 0.00]
Games | N: 628 W: 164 L: 160 D: 304
Penta | [13, 67, 147, 77, 10]
https://gedas.pythonanywhere.com/test/1664/
```

### 7.0

Use PAQ1-based compression (compressor and loader derived from Crinkler)

4 threads: 4021 bytes (-68)
MD5: 1814f1618379cf0d1858f7492413da0b

1 thread: 3916 bytes (-78)
MD5: 1235762468796929557205fdc4de077f

1 thread:
```py
Input:       ./build/4kc (5176 bytes)
Mode:        SLOW
Base prob:   10
Calculating models... 256/256
Estimated:   3285.951 bytes
Models:      00:1 80:2 40:1 C0:3 20:0 A0:2 60:2 90:2 50:1 AC:3 29:2 B0:3 48:0 88:1 32:1 CA:3 5A:3 95:5
Compressed:  3272 bytes (63.21%)
Output:      ./build/4kc.paq (3301 bytes, weightmask 20820DFF)
```

4 threads:
```py
Input:       ./build/4kc (5343 bytes)
Mode:        SLOW
Base prob:   10
Calculating models... 256/256
Estimated:   3391.935 bytes
Models:      00:1 80:2 40:1 C0:3 20:0 A0:3 60:2 A9:5 30:0 90:2 50:1 88:1 2C:1 B8:3 94:3 49:3 0C:0
Compressed:  3378 bytes (63.22%)
Output:      ./build/4kc.paq (3406 bytes, weightmask 10441BFF)
```

### 7.1

Blackbox after PAQ

4 threads: 3980 bytes (-41)
MD5: d13561ab73074fc0fb5750d2b011cf0e

1 thread: 3903 bytes (-13)
MD5: 1b100a00987cc5f58a06ccc5504530e9

1 thread:
```py
Input:       ./build/4kc (5154 bytes)
Mode:        SLOW
Base prob:   10
Calculating models... 256/256
Estimated:   3273.560 bytes
Models:      00:1 80:2 40:1 C0:3 20:0 A0:2 60:2 50:1 90:2 30:0 B0:3 88:2 A9:4 5A:3 44:1 29:2 32:1 41:0 97:5
Compressed:  3258 bytes (63.21%)
Output:      ./build/4kc.paq (3288 bytes, weightmask 10408AFF)
```

4 threads:
```py
Input:       ./build/4kc (5298 bytes)
Mode:        SLOW
Base prob:   10
Calculating models... 256/256
Estimated:   3352.282 bytes
Models:      00:1 80:2 40:1 C0:3 20:0 A0:2 60:2 90:2 50:1 30:0 B0:3 88:1 58:2 32:1 4C:2 A4:2 41:0 A9:4 2D:2 9D:5
Compressed:  3334 bytes (62.93%)
Output:      ./build/4kc.paq (3365 bytes, weightmask 1040257E)
```

```py
info depth 1 score cp 15 lowerbound time 0 nodes 18 nps 325821
info depth 1 score cp 31 time 0 nodes 38 nps 331617 pv b1c3
info depth 2 score cp 17 time 0 nodes 114 nps 387130 pv b1c3
info depth 3 score cp 27 time 0 nodes 234 nps 623296 pv b1c3
info depth 4 score cp 17 time 0 nodes 382 nps 733482 pv b1c3
info depth 5 score cp 3 time 0 nodes 583 nps 840883 pv b1c3
info depth 6 score cp 15 time 0 nodes 902 nps 956815 pv b1c3
info depth 7 score cp 15 time 2 nodes 2412 nps 1129732 pv b1c3
info depth 8 score cp 17 time 2 nodes 3530 nps 1178739 pv b1c3
info depth 9 score cp 18 time 9 nodes 11122 nps 1195747 pv g1f3
info depth 10 score cp 3 upperbound time 13 nodes 16346 nps 1214526
info depth 10 score cp 17 time 23 nodes 31141 nps 1311878 pv g1f3
info depth 11 score cp 14 time 39 nodes 56666 nps 1427399 pv g1f3
info depth 12 score cp 14 time 101 nodes 173831 nps 1704769 pv e2e4
info depth 13 score cp 14 time 166 nodes 318662 nps 1909974 pv b1c3
info depth 14 score cp 14 time 219 nodes 431515 nps 1962484 pv b1c3
info depth 15 score cp 18 time 400 nodes 843116 nps 2104867 pv d2d4
info depth 16 score cp 10 time 858 nodes 1867815 nps 2175346 pv d2d4
info depth 17 score cp 11 time 1068 nodes 2320678 nps 2171312 pv d2d4
info depth 18 score cp 16 time 1867 nodes 4035050 nps 2160575 pv d2d4
info depth 19 score cp 16 time 2600 nodes 5609184 nps 2157259 pv d2d4
info depth 20 score cp 25 time 4038 nodes 8578993 nps 2124208 pv e2e4
info depth 21 score cp 32 time 5143 nodes 10912214 nps 2121737 pv e2e4
info depth 22 score cp 28 time 6460 nodes 13794915 nps 2135426 pv e2e4
13794915 nodes 2135413 nps
```

```py
Elo   | -13.98 +- 25.77 (95%)
SPRT  | 10.0+0.10s Threads=1 Hash=1MB
LLR   | -0.25 (-2.94, 2.94) [-5.00, 0.00]
Games | N: 348 W: 92 L: 106 D: 150
Penta | [10, 48, 76, 26, 14]
https://gedas.pythonanywhere.com/test/1672/
```

### 7.2

Bits in PAQ header

4 threads: 3974 bytes (-6)
MD5: 9e421edcb7326ce76d3f9d09261b5ecd

1 thread: 3897 bytes (-6)
MD5: 3c8594dd61006a812d7c0dca04178882

### 7.3

* Remove baseprob from header
* Remove hashbits from header
* Use hashmask directly in loader

4 threads: 3962 bytes (-12)
MD5: 9e421edcb7326ce76d3f9d09261b5ecd

1 thread: 3885 bytes (-12)
MD5: 63327769d1b5978e9ba4bdf188f0d306

### 7.4

Shift-out-zero to terminate weightmask loop

4 threads: 3959 bytes (-3)
MD5: 6aa79604c6f21d64d920bb95d0c57a7c

1 thread: 3882 bytes (-3)
MD5: 3ba588ccfc2b7cfa294f7c87de8a025b

### 7.5

Run bitwise blackbox

4 threads: 3952 bytes (-7)
MD5: b32f9fb90e0731a76e2a0a5ae0fa8dc5

1 thread: 3882 bytes (=)
MD5: 3ba588ccfc2b7cfa294f7c87de8a025b

4 threads:
```py
Input:       ./build/4kc (5298 bytes)
Mode:        SLOW
Base prob:   10
Calculating models... 256/256
Estimated:   3345.639 bytes
Models:      00:1 80:2 40:1 C0:3 20:0 A0:2 60:2 90:2 30:0 50:1 B0:3 88:1 4C:2 58:2 A4:2 41:0 32:1 A9:4 2D:3 97:4
Compressed:  3327 bytes 26611 bits (62.80%)
Output:      ./build/4kc.paq (3354 bytes, weightmask 104044FF)
```

1 thread:
```py
Input:       ./build/4kc (5154 bytes)
Mode:        SLOW
Base prob:   10
Calculating models... 256/256
Estimated:   3273.331 bytes
Models:      00:1 80:2 40:1 C0:3 20:0 A0:2 60:2 50:1 90:2 30:0 B0:3 88:2 44:1 29:2 5A:3 A9:4 41:0 97:5
Compressed:  3259 bytes 26066 bits (63.23%)
Output:      ./build/4kc.paq (3284 bytes, weightmask 108115FF)
```

### 7.6

Remove hash reduction

4 threads: 3940 bytes (-12)
MD5: c83a2b1860f9bbe1f86c94cd6eb48bd8

1 thread: 3868 bytes (-14)
MD5: 01afd6e8128911002d95c3c468ae0db8

4 threads:
```py
Input:       ./build/4kc (5298 bytes)
Mode:        SLOW
Base prob:   10
Calculating models... 256/256
Estimated:   3345.639 bytes
Models:      00:1 80:2 40:1 C0:3 20:0 A0:2 60:2 90:2 30:0 50:1 B0:3 88:1 4C:2 58:2 A4:2 41:0 32:1 A9:4 2D:3 97:4
Compressed:  3327 bytes 26613 bits (62.80%)
Output:      ./build/4kc.paq (3354 bytes, weightmask 104044FF)
```

1 thread:
```py
Input:       ./build/4kc (5154 bytes)
Mode:        SLOW
Base prob:   10
Calculating models... 256/256
Estimated:   3273.331 bytes
Models:      00:1 80:2 40:1 C0:3 20:0 A0:2 60:2 50:1 90:2 30:0 B0:3 88:2 44:1 29:2 5A:3 A9:4 41:0 97:5
Compressed:  3257 bytes 26049 bits (63.19%)
Output:      ./build/4kc.paq (3282 bytes, weightmask 108115FF)
```

### 7.7

ELF smashing

4 threads: 3917 bytes (-23)
MD5: b8a5a22f080fcd9ad4349fc3ea7b1f6a

1 thread: 3845 bytes (-23)
MD5: 818c1ab1987ad509af7cfab9d9378f7f

### 7.8

More ELF smashing

4 threads: 3910 bytes (-7)
MD5: 9b9001d1747d92170205b26b14060023

1 thread: 3845 bytes (-7)
MD5: bcf6d5773f8092dfab13dabcfee2d0f8

### 7.9

Smaller loader

4 threads: 3865 (-45)
MD5: 3e3b748dab9d5a7a2e0e75fbfbfd2682

1 thread: 3793 bytes (-52)
MD5: e982028d1df220ee34a41242aa653e64

### 7.10

Smaller loader

4 threads: 3852 (-13)
MD5: cab510227bc2c881074a414a148b8322

1 thread: 3780 bytes (-13)
MD5: bfdcf511ffc8a423de06af8048fe5107

### 7.11

No used check in compression

4 threads: 3848 (-4)
MD5: be8c26794625c2b86b65ff7bbb93661f

1 thread: 3776 bytes (-4)
MD5: d6703799cc49d6fb8850d9d22c1faa9f

### 7.12

Smaller loader

4 threads: 3847 (-1)
MD5: f37466bd721b7eeafd05c0e1c658a7bd

1 thread: 3775 bytes (-1)
MD5: 31741715ba70ce98aeb72b33924117d7

### 7.13

Use direct syscalls

4 threads: 3842 bytes (-5)
MD5: 44f93783945c3544a786d6d3c7c8782d

1 thread: 3771 bytes (-4)
MD5: 62b05da0be0364682d88258fb5b879bd

4 threads:
```py
Input:       ./build/4kc (5294 bytes)
Level:       2
Base prob:   10
Calculating models... 256/256
Estimated:   3340.883 bytes
Models:      00:1 80:2 40:1 C0:3 20:0 A0:2 60:2 90:2 50:1 30:0 B0:3 88:1 58:2 32:1 A4:3 4C:2 41:0 29:2 A9:4 95:4
Compressed:  3322 bytes 26569 bits (62.75%)
Output:      ./build/4kc.paq (3349 bytes, weightmask 104044FF)
```

1 thread:
```py
Input:       ./build/4kc (5150 bytes)
Level:       2
Base prob:   10
Calculating models... 256/256
Estimated:   3270.334 bytes
Models:      00:1 80:2 40:1 C0:3 20:0 A0:2 60:2 50:1 90:2 30:0 B0:3 88:2 44:0 41:1 5A:3 29:2 A9:4 97:5
Compressed:  3253 bytes 26021 bits (63.17%)
Output:      ./build/4kc.paq (3278 bytes, weightmask 108115FF)
```

### 7.14

Smaller loader

4 threads: 3827 (-15)
MD5: f37466bd721b7eeafd05c0e1c658a7bd

1 thread: 3756 bytes (-15)
MD5: 31741715ba70ce98aeb72b33924117d7

### 7.14

Smaller loader

4 threads: 3821 (-6)
MD5: 772bad501b8db9e09bc8e342b24d1e22

1 thread: 3756 bytes (-6)
MD5: 629eb77b24a45d74c7133e165845708f

### 7.15

Smaller loader

4 threads: 3811 (-9)
MD5: ff81ae1d91413dededd66fc34c654c59

1 thread: 3740 bytes (-16)
MD5: 16001b49686c9092f712ad4c72d24a42

### 7.16

Rerun blackbox

4 threads: 3808 bytes (-3)
MD5: bc3d15d3ad29d196da075368fbdf55a2

1 thread: 3738 bytes (-2)
MD5: cb510790f9988ec7eaece619feaf25dc

4 threads:
```py
Input:       ./build/4kc (5294 bytes)
Level:       2
Base prob:   10
Calculating models... 256/256
Estimated:   3338.459 bytes
Models:      00:1 80:2 40:1 C0:3 20:0 A0:3 60:2 90:2 50:1 30:0 B0:3 88:2 4C:2 5A:3 29:2 97:5 41:0
Compressed:  3346 bytes 26767 bits (63.20%)
Output:      ./build/4kc.paq (3346 bytes)
```

1 thread:
```py
Input:       ./build/4kc (5154 bytes)
Level:       2
Base prob:   10
Calculating models... 256/256
Estimated:   3268.687 bytes
Models:      00:1 80:2 40:1 C0:3 20:0 A0:2 60:2 90:2 50:1 30:0 B0:3 88:1 44:1 29:2 A9:4 A4:2 5A:4 95:4
Compressed:  3276 bytes 26206 bits (63.56%)
Output:      ./build/4kc.paq (3276 bytes)
```

```py
info depth 1 score cp 15 lowerbound time 0 nodes 18 nps 192338
info depth 1 score cp 31 time 0 nodes 38 nps 239159 pv b1c3
info depth 2 score cp 17 time 0 nodes 114 nps 338710 pv b1c3
info depth 3 score cp 27 time 0 nodes 234 nps 563278 pv b1c3
info depth 4 score cp 17 time 0 nodes 382 nps 676336 pv b1c3
info depth 5 score cp 3 time 0 nodes 583 nps 768319 pv b1c3
info depth 6 score cp 15 time 1 nodes 902 nps 885403 pv b1c3
info depth 7 score cp 15 time 2 nodes 2412 nps 1089636 pv b1c3
info depth 8 score cp 17 time 3 nodes 3530 nps 1136026 pv b1c3
info depth 9 score cp 18 time 9 nodes 11122 nps 1133728 pv g1f3
info depth 10 score cp 3 upperbound time 14 nodes 16346 nps 1148121
info depth 10 score cp 17 time 25 nodes 31141 nps 1235102 pv g1f3
info depth 11 score cp 14 time 41 nodes 56666 nps 1365757 pv g1f3
info depth 12 score cp 14 time 102 nodes 173831 nps 1688034 pv e2e4
info depth 13 score cp 14 time 165 nodes 318662 nps 1923852 pv b1c3
info depth 14 score cp 14 time 217 nodes 431515 nps 1984243 pv b1c3
info depth 15 score cp 18 time 395 nodes 843116 nps 2133521 pv d2d4
info depth 16 score cp 10 time 841 nodes 1867815 nps 2219880 pv d2d4
info depth 17 score cp 11 time 1041 nodes 2320678 nps 2229171 pv d2d4
info depth 18 score cp 16 time 1809 nodes 4035050 nps 2230476 pv d2d4
info depth 19 score cp 16 time 2527 nodes 5609184 nps 2219559 pv d2d4
info depth 20 score cp 25 time 3879 nodes 8578993 nps 2211374 pv e2e4
info depth 21 score cp 32 time 4933 nodes 10912214 nps 2212058 pv e2e4
info depth 22 score cp 28 time 6286 nodes 13794915 nps 2194367 pv e2e4
13794915 nodes 2194348 nps
```

### 7.17

King shield evaluation

4 threads: 3860 bytes (+52)
MD5: 8f7ba0c48245f8d4b6ef4b5c3c5d6fa6

1 thread: 3809 bytes (+70)
MD5: 1d360e10f7aecf37b0799e607abc77ce

4 threads:
```py
Input:       ./build/4kc (5398 bytes)
Level:       2
Base prob:   10
Calculating models... 256/256
Estimated:   3390.306 bytes
Models:      00:1 80:2 40:1 C0:3 20:0 A0:2 60:2 90:2 50:1 30:0 B0:3 88:2 23:2 A4:2 5A:3 96:3 A9:5
Compressed:  3398 bytes 27184 bits (62.95%)
Output:      ./build/4kc.paq (3398 bytes)
```

1 thread:
```py
Input:       ./build/4kc (5279 bytes)
Level:       2
Base prob:   10
Calculating models... 256/256
Estimated:   3339.190 bytes
Models:      00:1 80:2 40:1 C0:3 20:0 A0:2 60:2 90:2 30:0 50:1 B0:3 88:1 48:0 A9:5 84:1 21:1 5A:3 CA:4
Compressed:  3347 bytes 26776 bits (63.40%)
Output:      ./build/4kc.paq (3347 bytes)
```

```py
info depth 1 score cp 15 lowerbound time 0 nodes 18 nps 257731
info depth 1 score cp 28 time 0 nodes 38 nps 292573 pv b1c3
info depth 2 score cp 17 time 0 nodes 118 nps 357199 pv b1c3
info depth 3 score cp 27 time 0 nodes 238 nps 550132 pv b1c3
info depth 4 score cp 17 time 0 nodes 413 nps 620573 pv b1c3
info depth 5 score cp 8 time 1 nodes 669 nps 663739 pv b1c3
info depth 6 score cp 14 time 1 nodes 1112 nps 750564 pv b1c3
info depth 7 score cp 15 time 4 nodes 3776 nps 788267 pv g1f3
info depth 8 score cp 16 time 9 nodes 6171 nps 650681 pv g1f3
info depth 9 score cp 11 time 36 nodes 22689 nps 615552 pv g1f3
info depth 10 score cp 11 time 71 nodes 51363 nps 713487 pv g1f3
info depth 11 score cp 17 time 117 nodes 86799 nps 740000 pv g1f3
info depth 12 score cp 13 time 182 nodes 159323 nps 874626 pv g1f3
info depth 13 score cp 17 time 290 nodes 293142 nps 1008074 pv g1f3
info depth 14 score cp 17 time 483 nodes 512314 nps 1060219 pv g1f3
info depth 15 score cp 18 time 806 nodes 903249 nps 1120434 pv g1f3
info depth 16 score cp 18 time 1275 nodes 1478273 nps 1159154 pv g1f3
info depth 17 score cp 20 time 2443 nodes 2847032 nps 1165025 pv e2e4
info depth 18 score cp 16 time 3653 nodes 4212762 nps 1153181 pv e2e4
info depth 19 score cp 26 time 4613 nodes 5341480 nps 1157770 pv e2e4
info depth 20 score cp 29 time 6281 nodes 7073524 nps 1126059 pv e2e4
info depth 21 score cp 14 upperbound time 10835 nodes 11396915 nps 1051777
info depth 21 score cp 35 time 12682 nodes 13474917 nps 1062465 pv e2e4
info depth 22 score cp 26 time 14665 nodes 15707862 nps 1071059 pv e2e4
15707862 nodes 1071052 nps
```

```py
Elo   | 14.17 +- 7.40 (95%)
SPRT  | 10.0+0.10s Threads=1 Hash=1MB
LLR   | 3.00 (-2.94, 2.94) [0.00, 5.00]
Games | N: 3998 W: 1190 L: 1027 D: 1781
Penta | [97, 436, 810, 519, 137]
https://gedas.pythonanywhere.com/test/1678/
```

```py
Elo   | 13.46 +- 6.97 (95%)
SPRT  | 60.0+0.60s Threads=1 Hash=1MB
LLR   | 2.96 (-2.94, 2.94) [0.00, 5.00]
Games | N: 3770 W: 1093 L: 947 D: 1730
Penta | [54, 424, 815, 506, 86]
https://gedas.pythonanywhere.com/test/1679/
```

### 7.18

Pawn threat evaluation

4 threads: 3885 bytes (+25)
MD5: d04d92bc864ad04cdd5812dbd3c0a539

1 thread: 3831 bytes (+22)
MD5: 7fa49f176ccf53d17bb882676ead36a9

4 threads:
```py
Input:       ./build/4kc (5423 bytes)
Level:       2
Base prob:   10
Calculating models... 256/256
Estimated:   3414.787 bytes
Models:      00:1 80:2 40:1 C0:3 20:0 A0:3 60:2 90:2 10:0 50:1 22:0 B0:3 88:2 58:2 44:1 31:2 A9:4 97:5
Compressed:  3423 bytes 27378 bits (63.12%)
Output:      ./build/4kc.paq (3423 bytes)
```

1 thread:
```py
Input:       ./build/4kc (5300 bytes)
Level:       2
Base prob:   10
Calculating models... 256/256
Estimated:   3360.770 bytes
Models:      00:1 80:2 40:1 C0:3 20:0 A0:3 60:2 90:2 10:0 50:1 22:0 B0:2 88:2 84:1 31:2 58:2 44:1 A9:4 D7:6
Compressed:  3369 bytes 26949 bits (63.57%)
Output:      ./build/4kc.paq (3369 bytes)
```

```py
info depth 1 score cp 14 time 0 nodes 13 nps 246487 pv e2e3
info depth 2 score cp 17 time 0 nodes 117 nps 301884 pv b1c3
info depth 3 score cp 26 time 0 nodes 262 nps 489143 pv b1c3
info depth 4 score cp 17 time 0 nodes 525 nps 666802 pv b1c3
info depth 5 score cp 15 time 1 nodes 847 nps 678181 pv b1c3
info depth 6 score cp 11 time 5 nodes 4273 nps 730656 pv d2d4
info depth 7 score cp 13 time 11 nodes 7580 nps 655800 pv d2d4
info depth 8 score cp 17 time 21 nodes 11881 nps 558654 pv d2d4
info depth 9 score cp 16 time 35 nodes 21008 nps 595448 pv d2d4
info depth 10 score cp 14 time 79 nodes 56708 nps 716264 pv g1f3
info depth 11 score cp 20 time 156 nodes 119886 nps 765814 pv g1f3
info depth 12 score cp 16 time 230 nodes 202700 nps 880342 pv g1f3
info depth 13 score cp 19 time 543 nodes 543976 nps 1001054 pv c2c4
info depth 14 score cp 14 time 909 nodes 1004473 nps 1104472 pv b1c3
info depth 15 score cp 23 time 1418 nodes 1649527 nps 1162936 pv d2d4
info depth 16 score cp 19 time 1893 nodes 2163064 nps 1142212 pv d2d4
info depth 17 score cp 19 time 2820 nodes 3259892 nps 1155932 pv d2d4
info depth 18 score cp 16 time 4076 nodes 4761422 nps 1168131 pv d2d4
info depth 19 score cp 26 time 5913 nodes 6911930 nps 1168748 pv d2d4
info depth 20 score cp 15 time 8881 nodes 10459927 nps 1177664 pv d2d4
info depth 21 score cp 15 time 12366 nodes 14328211 nps 1158661 pv d2d4
info depth 22 score cp 27 time 18680 nodes 21365945 nps 1143751 pv d2d4
21365945 nodes 1143745 nps
```

```py
Elo   | 9.15 +- 5.73 (95%)
SPRT  | 10.0+0.10s Threads=1 Hash=1MB
LLR   | 2.96 (-2.94, 2.94) [0.00, 5.00]
Games | N: 6498 W: 1857 L: 1686 D: 2955
Penta | [149, 772, 1277, 861, 190]
https://gedas.pythonanywhere.com/test/1686/
```

```py
Elo   | 9.21 +- 5.58 (95%)
SPRT  | 60.0+0.60s Threads=1 Hash=1MB
LLR   | 2.94 (-2.94, 2.94) [0.00, 5.00]
Games | N: 5810 W: 1563 L: 1409 D: 2838
Penta | [91, 671, 1249, 781, 113]
https://gedas.pythonanywhere.com/test/1687/
```

### 7.19

Bishop colour pawn evaluation

4 threads: 3930 bytes (+45)
MD5: 508ded8194fd8d089cf2d79b13d0454a

1 thread: 3871 bytes (+40)
MD5: d7ca60377b122582d25daa92e07f02f0

4 threads:
```py
Input:       ./build/4kc (5515 bytes)
Level:       2
Base prob:   10
Calculating models... 256/256
Estimated:   3459.014 bytes
Models:      00:1 80:2 40:1 C0:3 20:0 A0:2 60:2 90:2 10:0 50:1 B0:3 88:2 A4:3 A9:4 23:2 5A:3 D7:5
Compressed:  3468 bytes 27737 bits (62.88%)
Output:      ./build/4kc.paq (3468 bytes)
```

1 thread:
```py
Input:       ./build/4kc (5392 bytes)
Level:       2
Base prob:   10
Calculating models... 256/256
Estimated:   3399.803 bytes
Models:      00:1 80:2 40:1 C0:3 20:0 A0:2 60:1 90:2 30:0 50:1 B0:3 88:2 48:1 D7:6 A9:4 22:0 5A:4
Compressed:  3409 bytes 27269 bits (63.22%)
Output:      ./build/4kc.paq (3409 bytes)
```

```py
info depth 1 score cp 15 lowerbound time 0 nodes 18 nps 217079
info depth 1 score cp 26 time 0 nodes 38 nps 214805 pv b1c3
info depth 2 score cp 17 time 0 nodes 119 nps 290105 pv b1c3
info depth 3 score cp 23 time 0 nodes 239 nps 464610 pv b1c3
info depth 4 score cp 17 time 0 nodes 381 nps 543792 pv b1c3
info depth 5 score cp 10 time 0 nodes 604 nps 622766 pv b1c3
info depth 6 score cp 11 time 1 nodes 1182 nps 756719 pv b1c3
info depth 7 score cp 14 time 5 nodes 4018 nps 722806 pv b1c3
info depth 8 score cp 17 time 13 nodes 7806 nps 575818 pv b1c3
info depth 9 score cp 21 time 34 nodes 21887 nps 633543 pv d2d4
info depth 10 score cp 17 time 59 nodes 41584 nps 694038 pv d2d4
info depth 11 score cp 21 time 105 nodes 83753 nps 793063 pv d2d4
info depth 12 score cp 16 time 177 nodes 150403 nps 848293 pv d2d4
info depth 13 score cp 14 time 486 nodes 463607 nps 953866 pv d2d4
info depth 14 score cp 17 time 731 nodes 762678 nps 1043274 pv d2d4
info depth 15 score cp 18 time 1091 nodes 1153316 nps 1056782 pv d2d4
info depth 16 score cp 21 time 1706 nodes 1843033 nps 1079855 pv d2d4
info depth 17 score cp 16 time 2491 nodes 2646706 nps 1062338 pv d2d4
info depth 18 score cp 15 time 4431 nodes 4720224 nps 1065196 pv d2d4
info depth 19 score cp 28 time 6162 nodes 6566117 nps 1065571 pv d2d4
info depth 20 score cp 22 time 8319 nodes 9056617 nps 1088630 pv d2d4
info depth 21 score cp 24 time 11021 nodes 12236906 nps 1110295 pv d2d4
info depth 22 score cp 22 time 19314 nodes 21879843 nps 1132799 pv d2d4
21879843 nodes 1132797 nps
```

```py
Elo   | 10.29 +- 6.08 (95%)
SPRT  | 10.0+0.10s Threads=1 Hash=1MB
LLR   | 3.00 (-2.94, 2.94) [0.00, 5.00]
Games | N: 5604 W: 1631 L: 1465 D: 2508
Penta | [131, 607, 1197, 699, 168]
https://gedas.pythonanywhere.com/test/1689/
```

```py
Elo   | 5.07 +- 3.75 (95%)
SPRT  | 60.0+0.60s Threads=1 Hash=1MB
LLR   | 2.95 (-2.94, 2.94) [0.00, 5.00]
Games | N: 12192 W: 3181 L: 3003 D: 6008
Penta | [166, 1447, 2746, 1517, 220]
https://gedas.pythonanywhere.com/test/1690/
```

### 7.20

Reduce size in eval

4 threads: 3925 bytes (-5)
MD5: 60a18f0c3ea3939474962c91f2b32836

1 thread: 3869 bytes (-2)
MD5: 4ac7dc248c8db24505ede1198a965a44

4 threads:
```py
Input:       ./build/4kc (5504 bytes)
Level:       2
Base prob:   10
Calculating models... 256/256
Estimated:   3455.063 bytes
Models:      00:1 80:2 40:1 C0:3 20:0 A0:2 60:2 90:2 10:0 50:1 B0:3 88:2 5A:3 23:2 A4:3 A9:4 9D:5
Compressed:  3463 bytes 27702 bits (62.92%)
Output:      ./build/4kc.paq (3463 bytes)
```

1 thread:
```py
Input:       ./build/4kc (5381 bytes)
Level:       2
Base prob:   10
Calculating models... 256/256
Estimated:   3398.469 bytes
Models:      00:1 80:2 40:1 C0:3 20:0 A0:2 60:1 90:2 30:0 50:1 B0:3 88:2 48:1 9D:5 A9:4 22:0 5A:3 23:1
Compressed:  3407 bytes 27250 bits (63.32%)
Output:      ./build/4kc.paq (3407 bytes)
```

```py
info depth 1 score cp 15 lowerbound time 0 nodes 18 nps 360425
info depth 1 score cp 26 time 0 nodes 38 nps 368688 pv b1c3
info depth 2 score cp 17 time 0 nodes 119 nps 445988 pv b1c3
info depth 3 score cp 23 time 0 nodes 239 nps 689237 pv b1c3
info depth 4 score cp 17 time 0 nodes 381 nps 792434 pv b1c3
info depth 5 score cp 10 time 0 nodes 604 nps 898614 pv b1c3
info depth 6 score cp 11 time 1 nodes 1182 nps 1094702 pv b1c3
info depth 7 score cp 14 time 3 nodes 4018 nps 1061916 pv b1c3
info depth 8 score cp 17 time 7 nodes 7806 nps 1042132 pv b1c3
info depth 9 score cp 21 time 19 nodes 21887 nps 1131292 pv d2d4
info depth 10 score cp 17 time 34 nodes 41584 nps 1202583 pv d2d4
info depth 11 score cp 21 time 61 nodes 83753 nps 1369402 pv d2d4
info depth 12 score cp 16 time 96 nodes 150403 nps 1553900 pv d2d4
info depth 13 score cp 14 time 242 nodes 463607 nps 1909956 pv d2d4
info depth 14 score cp 17 time 376 nodes 762678 nps 2023689 pv d2d4
info depth 15 score cp 18 time 556 nodes 1153316 nps 2072960 pv d2d4
info depth 16 score cp 21 time 878 nodes 1843033 nps 2097195 pv d2d4
info depth 17 score cp 16 time 1265 nodes 2646706 nps 2091107 pv d2d4
info depth 18 score cp 15 time 2256 nodes 4720224 nps 2091848 pv d2d4
info depth 19 score cp 28 time 3180 nodes 6566117 nps 2064308 pv d2d4
info depth 20 score cp 22 time 4395 nodes 9056617 nps 2060432 pv d2d4
info depth 21 score cp 24 time 6005 nodes 12236906 nps 2037762 pv d2d4
info depth 22 score cp 22 time 10967 nodes 21879843 nps 1995045 pv d2d4
21879843 nodes 1995037 nps
```
