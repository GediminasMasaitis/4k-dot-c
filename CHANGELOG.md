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

```
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

```
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

```
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

```
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

```
Score of 4k.c-0.40 vs 4k.c-0.39: 522 - 458 - 1023  [0.516] 2003
...      4k.c-0.40 playing White: 310 - 181 - 511  [0.564] 1002
...      4k.c-0.40 playing Black: 212 - 277 - 512  [0.468] 1001
...      White vs Black: 587 - 393 - 1023  [0.548] 2003
Elo difference: 11.1 +/- 10.6, LOS: 98.0 %, DrawRatio: 51.1 %
```
