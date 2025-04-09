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
Elo   | 15.98 +- 17.26 (95%)
SPRT  | 60.0+0.60s Threads=1 Hash=1MB
LLR   | 0.69 (-2.94, 2.94) [0.00, 5.00]
Games | N: 892 W: 302 L: 261 D: 329
Penta | [31, 95, 167, 108, 45]
https://gedas.pythonanywhere.com/test/514/
```
