| Version | Change | Bytes | Delta | Elo (10+0.1) | Elo (60+0.6) |
|---|---|---|---|---|---|
| 0.1 | Hello world, thanks to zamfofex | 171 |  |  |  |
| 0.2 | Start of UCI | 335 | +164 |  |  |
| 0.3 | 64 bit binary, thanks to zamfofex | 414 |  |  |  |
| 0.4 | Port 4ku base chess | 3872 | +3458 |  |  |
| 0.5 | All static functions, gcc 14.2 | 2528 | -1344 |  |  |
| 0.6 | UCI compliant engine which prints the first move | 3776 | +1248 |  |  |
| 0.7 | Add perft | 3776 | = |  |  |
| 0.8 | Perft NPS | 3776 | = |  |  |
| 0.9 | Remove hyperbola quintessence | 3217 | -559 |  |  |
| 0.10 | Fix legal movegen | 3313 | +96 |  |  |
| 0.11 | Material evaluation | 3490 | +117 | +29.9 +/- 1.9 |  |
| 0.12 | Fix evaluation | 3498 | -8 | +4.8 +/- 1.0 |  |
| 0.13 | Fixed-depth = 3 search | 3513 | +15 | +225.9 +/- 5.0 |  |
| 0.14 | Fix search | 3513 | = |  |  |
| 0.15 | Mate and stalemate dectection | 3577 | +64 | +24.9 +/- 3.7 |  |
| 0.16 | Iterative deepening | 3804 | +227 | +81.4 +/- 13.5 |  |
| 0.17 | Alpha-beta pruning | 3856 | +52 | +56.1 +/- 15.7 |  |
| 0.18 | Centrality evaluation | 3988 | +132 | +56.1 +/- 16.7 |  |
| 0.19 | Fail-hard approach | 3966 | -22 | +7.4 +/- 12.7 |  |
| 0.20 | Haswell arch build | 3944 | -22 | +3.9 +/- 14.0 |  |
| 0.21 | Reduce eval constant size | 3904 | -40 | -0.3 +/- 15.6 |  |
| 0.22 | Split quantized PST evaluation | 3967 | +63 | +22.3 +/- 12.1 |  |
| 0.23 | Size savings, thanks to kz04px | 3802 | -165 | +4.2 +/- 17.4 |  |
| 0.24 | Capture-first move ordering | 3898 | +92 | +38.0 +/- 17.6 |  |
| 0.25 | Depth-limited qsearch | 3994 | +96 | +88.2 +/- 9.4 |  |
| 0.26 | Best-ply-move ordering | 4026 | +32 | +55.4 +/- 15.0 |  |
| 0.27 | Treat moves as 64-bit integers | 3994 | -32 |  |  |
| 0.28 | In-check extension | 3994 | = | +15.8 +/- 11.3 |  |
| 0.29 | Tempo evaluation | 3994 | = | +4.1 +/- 2.3 |  |
| 0.30 | Reduce bytes | 3962 | -32 |  |  |
| 0.31 | Optimize ray function | 3898 | -36 | +9.6 +/- 11.2 |  |
| 0.32 | Reduce bytes | 3858 | -40 |  |  |
| 0.33 | Fix UCI time parsing bug | 3842 | -16 | +156.8 +/- 17.7 |  |
| 0.34 | MVV move ordering | 3844 | +2 | +18.5 +/- 9.6 |  |
| 0.35 | Reverse futility pruning | 3856 | +12 | +23.0 +/- 15.3 |  |
| 0.36 | Fix node counting | 3856 | = |  |  |
| 0.37 | Full time management | 3952 | +96 | -12.9 +/- 34.0 |  |
| 0.38 | Smaller TM using globals | 3944 | -8 |  |  |
| 0.39 | Principal variation search | 4032 | +88 | +44.4 +/- 14.6 |  |
| 0.40 | Remove qsearch depth limit | 4024 | -8 | +11.1 +/- 10.6 |  |
| 0.41 | Smaller PVS | 4016 | -8 |  |  |
| 0.42 | Reduce size | 4000 | -16 |  |  |
| 0.43 | Reduce size | 3960 | -40 |  |  |
| 0.44 | Reduce size with UCI hacks | 3904 | -56 |  |  |
| 0.45 | Reduce size by abberviating authorship | 3888 | -16 |  |  |
| 0.46 | Reduce size | 3880 | -8 |  |  |
| 0.47 | No RFP in PV nodes | 3888 | +8 | +13.9 +/- 10.4 |  |
| 0.48 | Protect from ply overflow | 3904 | +16 |  |  |
| 0.49 | None = 0 | 3912 | +8 |  |  |
| 0.50 | History heuristic | 4088 | +176 | +43.8 +/- 11.5 |  |
| 0.51 | Reduce bytes and slight speedup | 4080 | -8 | +11.6 +/- 8.4 |  |
| 0.52 | Reduce bytes | 4072 | -8 |  |  |
| 0.53 | Slight speedup | 4072 | = |  |  |
| 0.54 | Reduce bytes | 4048 | -24 | +4.5 +/- 10.4 |  |
| 0.55 | Less timing syscalls | 4048 | = | +3.3 +/- 10.3 |  |
| 0.56 | Reduce bytes | 4040 | -8 |  |  |
| 0.57 | Reduce bytes | 3976 | -64 |  |  |
| 0.58 | Late move reduction | 4048 | +72 | +41.5 +/- 14.0 |  |
| 0.59 | Slight speedup | 4048 | = |  |  |
| 0.60 | Reduce bytes by assuming stdin never closes | 4024 | -16 |  |  |
| 0.61 | Reduce bytes by removing UCI info | 4000 | -24 |  |  |
| 0.62 | Reduce bytes by smaller material table | 3992 | -8 |  |  |
| 0.63 | Reduce bytes by changing C standard to gnu23 | 3984 | -8 |  |  |
| 0.64 | Partial repetition detection | 4080 | +88 | +27.4 +/- 11.2 |  |
| 0.65 | Reduce size by removing custom bool definition | 4064 | -16 |  |  |
| 0.66 | LMR adjustment by move count | 4056 | -8 | +3.4 +/- 2.6 |  |
| 0.67 | Reduce size by moving position history | 4032 | -24 |  |  |
| 0.68 | Reduce size by using a search stack | 4016 | -16 |  |  |
| 0.69 | Reduce size by offloading move list to search stack | 3992 | -24 | +95.8 +/- 15.3 |  |
| 0.70 | Reduce size by restricting move string | 3984 | -8 | -5.9 +/- 15.7 |  |
| 0.71 | Reduce size by restricting every pointer possible | 3968 | -16 | +2.4 +/- 15.4 |  |
| 0.72 | Full repetition detection | 4048 | +80 | +18.7 +/- 8.0 |  |
| 0.73 | No repetition detection on negative depths | 4056 | +8 | +30.4 +/- 2.2 |  |
| 0.74 | Retune eval, remove quantization | 4064 | +8 | +34.6 +/- 14.7 |  |
| 0.75 | Do LMR earlier | 4064 | = | +17.6 +/- 7.6 |  |
| 0.76 | Adjust LMR by being in a zero-window | 4072 | +8 | +4.2 +/- 2.0 |  |
| 0.77 | Reduce size by reordering position struct | 4032 | -40 | [-9.52 +/- 8.45](https://gedas.pythonanywhere.com/test/40/) |  |
| 0.78 | Sided history table | 4056 | +16 | [+16.12 +/- 8.58](https://gedas.pythonanywhere.com/test/42/) |  |
| 0.78 | Set min time control to /48 | 4056 | = | [+11.04 +/- 6.34](https://gedas.pythonanywhere.com/test/45/) |  |
| 0.79 | Set min time control to /32, max time control to /2 | 4056 | = | [+9.35 +/- 6.09](https://gedas.pythonanywhere.com/test/48/) |  |
| 0.80 | Reduce size by optimizing history updating | 4048 | -8 |  |  |
| 0.81 | Noticed some losses on time so revert super aggressive time control | 4056 | +8 |  |  |
| 0.82 | Reduce size by removing blockers for king | 4048 | -8 |  |  |
| 0.83 | Retune eval for 16 tempo | 4048 | = | [+4.84 +/- 3.84](https://gedas.pythonanywhere.com/test/53/) |  |
| 0.84 | Reverse bitboards | 4096 | +48 | [+43.28 +/- 14.57](https://gedas.pythonanywhere.com/test/115/) |  |
| 0.85 | Retune eval with K=2.7 | 4096 | = | [+4.44 +/- 3.54](https://gedas.pythonanywhere.com/test/122/) |  |
| 0.86 | Reduce bytes by not flipping position in eval | 4080 | -16 | [-4.54 +/- 3.49](https://gedas.pythonanywhere.com/test/128/) |  |
| 0.87 | Reduce size with a hack for position flipping | 4072 | -8 | [-1.47 +/- 1.73](https://gedas.pythonanywhere.com/test/129/) |  |
| 0.88 | Reduce size by printing char by char | 4064 | -8 |  |  |
| 0.89 | Store capture inside move struct | 4096 | +24 | [+47.26 +/- 15.24](https://gedas.pythonanywhere.com/test/142/) |  |
| 0.90 | Killer heuristic and size reductions to fit it | 4096 | = | [+16.14 +/- 8.60](https://gedas.pythonanywhere.com/test/165/) |  |
| 0.91 | Reduce size by rewriting piece movegen | 3976 | -120 |  |  |
| 0.92 | Forward futility pruning | 4064 | +96 | [+25.96 +/- 11.23](https://gedas.pythonanywhere.com/test/179/) |  |
| 0.93 | Slightly faster position comparison | 4064 | = | [+9.13 +/- 7.01](https://gedas.pythonanywhere.com/test/185/) |  |
| 0.94 | Use promotion material in forward futility pruning | 4080 | +16 | [+7.90 +/- 5.26](https://gedas.pythonanywhere.com/test/180/) |  |
| 0.95 | Reorder castling in movegen | 4072 | -8 | [+4.21 +/- 5.57](https://gedas.pythonanywhere.com/test/192/) |  |
| 0.96 | Allow castling in qsearch | 4064 | -8 | [+4.32 +/- 4.99](https://gedas.pythonanywhere.com/test/201/) |  |
| 0.97 | No repetition detection in qsearch | 4080 | +16 | [+15.67 +/- 8.47](https://gedas.pythonanywhere.com/test/213/) |  |
| 0.98 | Pawn double-moves first | 4064 | -16 | [-2.16 +/- 2.05](https://gedas.pythonanywhere.com/test/221/) |  |
| 0.99 | Bishop pair evaluation | 4088 | +24 | [+9.14 +/- 6.07](https://gedas.pythonanywhere.com/test/227/) |  |
| 0.100 | Smaller forward futility pruning | 4072 | -16 | [+1.23 +/- 3.63](https://gedas.pythonanywhere.com/test/281/) |  |
| 0.101 | No ply check for early exits | 4064 | -8 | [+0.89 +/- 3.54](https://gedas.pythonanywhere.com/test/283/) |  |
| 0.102 | Reset history on capture | 4080 | +16 | [+8.95 +/- 5.63](https://gedas.pythonanywhere.com/test/298/) |  |
| 0.103 | Make mate detection branchless | 4072 | -8 | [-0.72 +/- 2.94](https://gedas.pythonanywhere.com/test/301/) |  |
| 0.104 | Reverse futility pruning margin 48 | 4072 | = | [+7.35 +/- 5.22](https://gedas.pythonanywhere.com/test/288/) | [+3.04 +/- 3.98](https://gedas.pythonanywhere.com/test/290/) |
| 0.105 | Smaller moves evaluated | 4064 | -8 |  |  |
| 0.106 | Doubled pawn evaluation | 4096 | +32 | [+19.81 +/- 9.48](https://gedas.pythonanywhere.com/test/311/) |  |
| 0.107 | Allow FFP in QS | 4096 | = | [+7.28 +/- 5.26](https://gedas.pythonanywhere.com/test/326/) | [+3.18 +/- 4.29](https://gedas.pythonanywhere.com/test/328/) |
| 0.108 | Smaller mobility | 4096 | = | [+4.37 +/- 4.91](https://gedas.pythonanywhere.com/test/348/) |  |
| 0.109 | Smaller pawn movegen | 4088 | -8 | [+0.02 +/- 2.39](https://gedas.pythonanywhere.com/test/345/) |  |
| 0.110 | Use current master branch of GCC for build | 4080 | -8 | +11.1 +/- 12.4 |  |
| 0.111 | Extended doubled pawn eval | 4096 | +16 | [+3.03 +/- 2.28](https://gedas.pythonanywhere.com/test/350/) | [+5.57 +/- 8.07](https://gedas.pythonanywhere.com/test/351/) |
| 0.112 | Smaller piece takes | 4088 | -8 |  |  |
| 0.113 | More aggressive TM | 4088 | = | [+17.66 +/- 8.85](https://gedas.pythonanywhere.com/test/368/) | [+17.31 +/- 8.67](https://gedas.pythonanywhere.com/test/366/) |
| 0.114 | Open files evaluation, deleting bishop pair and doubled pawns | 4096 | +8 | [+24.17 +/- 10.85](https://gedas.pythonanywhere.com/test/372/) |  |
| 0.115 | Branchless open files | 4096 | = | [+4.61 +/- 3.49](https://gedas.pythonanywhere.com/test/376/) |  |
| 0.116 | Optimized move string comparison (Suggested by Zamfofex) | 4088 | -8 |  |  |
| 0.117 | Smaller promotions in makemove (Suggested by Zamfofex) | 4080 | -8 | [-0.08 +/- 2.26](https://gedas.pythonanywhere.com/test/383/) |  |
| 0.118 | Even smaller move string comparison (Suggested by swedishchef) | 4072 | -8 |  |  |
| 0.119 | Proper quit command support | 4088 | +16 |  |  |
| 0.120 | Increase line buffer for safety | 4088 | = |  |  |
| 0.121 | Don't include bss in binary | 4058 | -30 |  |  |
| 0.122 | Remove bss prevention hack | 4018 | -40 |  |  |
| 0.123 | Protected pawn eval | 4074 | +56 | [+14.78 +/- 8.22](https://gedas.pythonanywhere.com/test/403/) |  |
| 0.124 | Shrink movegen (Implemented by cj5716) | 4042 | -32 |  |  |
| 0.125 | Bishop pair eval | 4067 | +25 | [+9.42 +/- 6.26](https://gedas.pythonanywhere.com/test/405/) |  |
| 1.0 | Safety features | 4080 | +13 |  |  |
| 1.1 | Transposition table | 4083 | +3 | [+44.37 +/- 15.54](https://gedas.pythonanywhere.com/test/435/) | [+38.42 +/- 14.14](https://gedas.pythonanywhere.com/test/438/) |
| 1.2 | Final round of AES for bit mixing | 4092 | +9 | [+47.55 +/- 15.16](https://gedas.pythonanywhere.com/test/459/) |  |
| 1.3 | Use self as key for final AES round | 4088 | -4 | [+0.20 +/- 5.66](https://gedas.pythonanywhere.com/test/462/) |  |
| 1.4 | Smaller TT entry | 4087 | -1 | [+5.43 +/- 6.06](https://gedas.pythonanywhere.com/test/465/) |  |
| 1.5 | Various size reductions | 4077 | -10 |  |  |
| 1.6 | Reduce size by making make_move non-static | 4061 | -16 |  |  |
| 1.7 | Preserve TT move | 4081 | +20 | [+31.76 +/- 12.14](https://gedas.pythonanywhere.com/test/468/) |  |
| 1.8 | Smaller AES hashing | 4070 | -11 | [-0.68 +/- 2.48](https://gedas.pythonanywhere.com/test/481/) |  |
| 1.9 | Razoring | 4096 | +26 | [+39.67 +/- 13.65](https://gedas.pythonanywhere.com/test/501/) |  |
| 1.10 | Razoring margin 128 | 4096 | = | [+15.88 +/- 8.30](https://gedas.pythonanywhere.com/test/512/) | [+20.79 +/- 9.60](https://gedas.pythonanywhere.com/test/514/) |
| 1.11 | Smaller bestmove preservation | 4077 | -19 | [-7.85 +/- 5.45](https://gedas.pythonanywhere.com/test/517/) |  |
| 1.12 | Fix move comparison | 4050 | -27 | [+13.00 +/- 8.62](https://gedas.pythonanywhere.com/test/529/) |  |
| 1.13 | Add open file evaluation; Remove killer heuristic | 4093 | +43 | [+22.46 +/- 10.21](https://gedas.pythonanywhere.com/test/552/) | [-1.25 +/- 4.10](https://gedas.pythonanywhere.com/test/554/) |
| 1.14 | Doubled pawns through open files | 4096 | +3 | [+23.04 +/- 10.42](https://gedas.pythonanywhere.com/test/555/) |  |
| 1.15 | Simplified LMR | 4082 | -14 | [+30.61 +/- 12.64](https://gedas.pythonanywhere.com/test/559/) | [-4.84 +/- 5.83](https://gedas.pythonanywhere.com/test/560/) |
| 1.16 | Reduce size | 4075 | -7 |  |  |
| 1.17 | Killer heuristic | 4093 | +18 | [+21.27 +/- 9.84](https://gedas.pythonanywhere.com/test/564/) | [+53.62 +/- 15.81](https://gedas.pythonanywhere.com/test/565/) |
| 1.18 | Don't consider castling for hash | 4089 | -4 | [-0.70 +/- 2.41](https://gedas.pythonanywhere.com/test/575/) |  |
| 1.19 | Use search stack for move parsing | 4081 | -8 |  |  |
| 1.20 | Store move count on search stack | 4075 | -6 |  |  |
| 1.21 | Bishop pair eval | 4095 | +20 | [+10.51 +/- 6.56](https://gedas.pythonanywhere.com/test/581/) | [+13.31 +/- 7.40](https://gedas.pythonanywhere.com/test/583/) |
| 1.22 | 4 byte (32 bit) partial hash in TT entry | 4093 | -2 | [+4.09 +/- 5.34](https://gedas.pythonanywhere.com/test/589/) |  |
| 1.23 | RFP margin 42 | 4093 | = | [+4.35 +/- 3.45](https://gedas.pythonanywhere.com/test/603/) | [+4.54 +/- 3.59](https://gedas.pythonanywhere.com/test/604/) |
| 1.24 | 2 byte (16 bit) partial hash in TT entry | 4093 | = | [+1.23 +/- 1.78](https://gedas.pythonanywhere.com/test/590/) | [+15.37 +/- 8.08](https://gedas.pythonanywhere.com/test/605/) |
| 1.25 | Hash default 64 MB | 4093 | = | [+3.18 +/- 2.48](https://gedas.pythonanywhere.com/test/606/) | [+33.84 +/- 12.41](https://gedas.pythonanywhere.com/test/607/) |
| 1.26 | Smaller partial TT key computing | 4086 | -7 |  |  |
| 2.0 | Equivalent to version 1.26 |  |  |  |  |
| 2.1 | LZ4 compression | 3970 | -116 |  |  |
| 2.2 | Null move pruning | 4089 | +119 | [+70.69 +/- 17.99](https://gedas.pythonanywhere.com/test/615/) | [+77.57 +/- 19.29](https://gedas.pythonanywhere.com/test/623/) |
| 2.3 | NMP static eval check | 4095 | +6 | [+15.16 +/- 8.09](https://gedas.pythonanywhere.com/test/625/) |  |
| 2.4 | NMP from depth 2 | 4095 | = | [+19.90 +/- 9.31](https://gedas.pythonanywhere.com/test/627/) |  |
| 2.5 | Smaller loader | 4025 | -70 |  |  |
| 2.6 | Even smaller loader | 4010 | -15 |  |  |
| 2.7 | Allow repetition detection after null move | 4010 | = | [+6.42 +/- 6.17](https://gedas.pythonanywhere.com/test/636/) |  |
| 2.8 | Move history heuristic table to global variable | 3985 | -25 |  |  |
| 2.9 | History malus | 4039 | +54 | [+24.19 +/- 10.51](https://gedas.pythonanywhere.com/test/648/) | [+44.97 +/- 14.49](https://gedas.pythonanywhere.com/test/649/) |
| 2.10 | Smaller time management | 4034 | -5 |  |  |
| 2.11 | Late move pruning | 4092 | +58 | [+52.67 +/- 15.80](https://gedas.pythonanywhere.com/test/658/) | [+35.48 +/- 12.79](https://gedas.pythonanywhere.com/test/659/) |
| 3.0 | Equivalent to 2.11 |  |  |  |  |
| 3.1 | aPLib compression | 3870 | -222 |  |  |
| 3.2 | Smaller aPLib compression | 3859 | -11 |  |  |
| 3.3 | History gravity; Capture history | 3909 | +50 | [+22.80 +/- 10.15](https://gedas.pythonanywhere.com/test/670/) | [+44.33 +/- 13.79](https://gedas.pythonanywhere.com/test/671/) |
| 3.4 | Smaller history heuristic | 3905 | -4 |  |  |
| 3.5 | Fail-soft main search | 3944 | +39 | [+28.90 +/- 11.33](https://gedas.pythonanywhere.com/test/681/) | [+32.06 +/- 11.82](https://gedas.pythonanywhere.com/test/683/) |
| 3.6 | Static eval TT adjustment | 3979 | +35 | [+32.72 +/- 12.21](https://gedas.pythonanywhere.com/test/688/) | [+18.82 +/- 8.82](https://gedas.pythonanywhere.com/test/689/) |
| 3.7 | LMP base moves = 2 | 3978 | -1 | [+4.56 +/- 3.60](https://gedas.pythonanywhere.com/test/694/) | [+3.59 +/- 5.72](https://gedas.pythonanywhere.com/test/695/) |
| 3.8 | SPSA tune | 3985 | +7 | [+10.76 +/- 6.52](https://gedas.pythonanywhere.com/test/708/) | [+11.30 +/- 6.61](https://gedas.pythonanywhere.com/test/709/) |
| 3.9 | SPSA tune | 3982 | -3 | [+20.77 +/- 9.51](https://gedas.pythonanywhere.com/test/714/) | [+9.98 +/- 6.12](https://gedas.pythonanywhere.com/test/715/) |
| 3.10 | Mobility evaluation | 4049 | +67 | [+18.09 +/- 8.92](https://gedas.pythonanywhere.com/test/719/) | [+34.73 +/- 12.31](https://gedas.pythonanywhere.com/test/721/) |
| 3.11 | King attacks evaluation | 4066 | +17 | [+20.77 +/- 9.57](https://gedas.pythonanywhere.com/test/724/) | [+37.14 +/- 12.62](https://gedas.pythonanywhere.com/test/725/) |
| 3.12 | Smaller aPLib decompressor | 3999 | -67 |  |  |
| 3.13 | Even smaller aPLib decompressor | 3942 | -57 |  |  |
| 3.14 | No castling in qsearch | 3944 | +2 | [+2.73 +/- 5.40](https://gedas.pythonanywhere.com/test/742/) |  |
| 3.15 | LMP adjustment based on improving | 3987 | +43 | [+22.49 +/- 9.90](https://gedas.pythonanywhere.com/test/754/) | [+23.24 +/- 9.76](https://gedas.pythonanywhere.com/test/755/) |
| 3.16 | LMR adjustment based on: Zero window; Improving | 4000 | +13 | [+11.74 +/- 6.86](https://gedas.pythonanywhere.com/test/761/) | [+11.79 +/- 6.74](https://gedas.pythonanywhere.com/test/763/) |
| 3.17 | 3-step PVS | 4002 | +2 | [+18.08 +/- 8.70](https://gedas.pythonanywhere.com/test/767/) | [+13.67 +/- 16.59](https://gedas.pythonanywhere.com/test/769/) |
| 3.18 | Forward futility pruning | 4092 | +90 | [+21.12 +/- 9.65](https://gedas.pythonanywhere.com/test/779/) | [+23.19 +/- 9.72](https://gedas.pythonanywhere.com/test/781/) |
| 3.19 | Allow repetition detection in qsearch | 4087 | -5 | [+1.20 +/- 3.80](https://gedas.pythonanywhere.com/test/804/) | [+3.79 +/- 10.77](https://gedas.pythonanywhere.com/test/811/) |
| 3.20 | No repetition detection after null move | 4085 | -2 | [+1.20 +/- 3.80](https://gedas.pythonanywhere.com/test/804/) | [+3.79 +/- 10.77](https://gedas.pythonanywhere.com/test/811/) |
| 4.0 | Functionally equivalent to version 3.20 | 4085 | = |  |  |
| 4.1 | Use assembly version of aPLib decompressor | 3897 | -188 |  |  |
| 4.2 | Don't compute final size in aPLib | 3887 | -10 |  |  |
| 4.3 | Disable >32kb case in aPLib | 3880 | -7 |  |  |
| 4.4 | Smaller aPLib decompression | 3875 | -5 |  |  |
| 4.5 | Smaller IIR | 3859 | -16 |  |  |
| 4.6 | NMP depth adjust by depth | 3867 | +8 | [+9.73 +/- 6.10](https://gedas.pythonanywhere.com/test/861/) | [+8.07 +/- 5.29](https://gedas.pythonanywhere.com/test/862/) |
| 4.7 | LMR move divisor 11 | 3867 | = | [+7.25 +/- 5.07](https://gedas.pythonanywhere.com/test/863/) | [+4.02 +/- 3.21](https://gedas.pythonanywhere.com/test/864/) |
| 4.8 | Precompute history bonus | 3859 | -8 |  |  |
| 4.9 | Dedicated puts function | 3859 | = |  |  |
| 4.10 | Null move pruning fail soft | 3851 | -8 | [+0.66 +/- 3.43](https://gedas.pythonanywhere.com/test/888/) | [+5.24 +/- 5.66](https://gedas.pythonanywhere.com/test/890/) |
| 4.11 | Smaller pruning code | 3843 | -8 |  |  |
| 4.12 | No padding for compressed payload | 3835 | -8 |  |  |
| 4.13 | Remove padding fill for loader | 3833 | -2 |  |  |
| 4.14 | Tapered evaluation; No forward futility pruning | 4068 | +235 | [+63.48 +/- 17.90](https://gedas.pythonanywhere.com/test/907/) | [+85.86 +/- 20.03](https://gedas.pythonanywhere.com/test/908/) |
| 4.15 | Smaller eval structs | 4053 | -15 |  |  |
| 4.16 | Smaller and faster mobility | 4049 | -4 | [+18.17 +/- 9.58](https://gedas.pythonanywhere.com/test/910/) |  |
| 4.17 | Smaller TT move | 4041 | -8 | [+8.62 +/- 6.92](https://gedas.pythonanywhere.com/test/926/) |  |
| 4.18 | Smaller is_attacked | 4030 | -11 | [+0.49 +/- 3.24](https://gedas.pythonanywhere.com/test/929/) |  |
| 4.19 | Smaller attack functions | 4012 | -18 | [+2.14 +/- 4.26](https://gedas.pythonanywhere.com/test/932/) |  |
| 4.20 | Forward futility pruning | 4090 | +78 | [+23.25 +/- 10.13](https://gedas.pythonanywhere.com/test/936/) |  |
| 4.21 | Automatic code reordering | 4053 | -37 | [+0.61 +/- 3.25](https://gedas.pythonanywhere.com/test/966/) | [+2.62 +/- 4.33](https://gedas.pythonanywhere.com/test/967/) |
| 4.22 | More code reordering | 4023 | -30 | [-3.78 +/- 11.17](https://gedas.pythonanywhere.com/test/968/) |  |
| 4.23 | Correlated reordering | 4012 | -11 | [+1.23 +/- 3.68](https://gedas.pythonanywhere.com/test/970/) |  |
| 4.24 | Material combining size decrease | 4008 | -4 |  |  |
| 4.25 | Multi runs with group shuffling | 3993 | -15 | [-0.37 +/- 2.55](https://gedas.pythonanywhere.com/test/978/) |  |
| 4.26 | Nested groups | 3970 | -23 | [+4.58 +/- 7.81](https://gedas.pythonanywhere.com/test/982/) |  |
| 4.27 | More nested groups | 3968 | -2 |  |  |
| 4.28 | Passed pawn evaluation | 4054 | +86 | [+36.11 +/- 12.68](https://gedas.pythonanywhere.com/test/992/) | [+22.13 +/- 9.72](https://gedas.pythonanywhere.com/test/993/) |
| 4.29 | Adjust max material | 4047 | -7 | [+3.39 +/- 4.92](https://gedas.pythonanywhere.com/test/995/) | [+5.76 +/- 12.72](https://gedas.pythonanywhere.com/test/997/) |
| 4.30 | Mobility without opponent pawn attack | 4078 | +31 | [+13.39 +/- 7.44](https://gedas.pythonanywhere.com/test/1012/) |  |
| 4.31 | Reordering to reduce size | 4063 | -15 |  |  |
| 4.32 | More ordering groups | 4061 | -2 |  |  |
| 4.33 | 4 hours reordering | 4048 | -13 |  |  |
| 4.34 | Blocked passed pawns evaluation | 4093 | +45 | [+23.06 +/- 9.89](https://gedas.pythonanywhere.com/test/1018/) | [+18.60 +/- 8.82](https://gedas.pythonanywhere.com/test/1019/) |
| 4.35 | Reduce size by reordering | 4085 | -8 |  |  |
| 4.36 | Knight mobility | 4084 | -1 | [+9.09 +/- 5.81](https://gedas.pythonanywhere.com/test/1046/) |  |
| 4.37 | Static toggling | 4069 | -15 |  |  |
| 4.38 | More passers | 4068 | -1 | [+1.94 +/- 1.80](https://gedas.pythonanywhere.com/test/1069/) |  |
| 4.39 | Attacked by pawns penalty | 4096 | +28 | [+14.95 +/- 7.83](https://gedas.pythonanywhere.com/test/1072/) |  |
| 4.40 | Smaller best move finding | 4092 | -4 |  |  |
| 4.41 | Tune parameters | 4094 | +2 | [+2.62 +/- 1.94](https://gedas.pythonanywhere.com/test/1094/) | [+2.32 +/- 1.65](https://gedas.pythonanywhere.com/test/1095/) |
| 4.42 | Include castling permissions into hash | 4087 | -7 | [-0.07 +/- 2.75](https://gedas.pythonanywhere.com/test/1120/) | [-0.29 +/- 2.47](https://gedas.pythonanywhere.com/test/1121/) |
| 4.43 | Smaller decompressor | 4084 | -3 |  |  |
| 4.44 | Tune parameters | 4083 | -1 | [+2.54 +/- 1.88](https://gedas.pythonanywhere.com/test/1135/) | [+2.67 +/- 2.07](https://gedas.pythonanywhere.com/test/1136/) |
| 4.46 | Faster move ordering | 4094 | +11 | [+9.28 +/- 5.88](https://gedas.pythonanywhere.com/test/1149/) | [+9.19 +/- 5.68](https://gedas.pythonanywhere.com/test/1150/) |
| 4.47 | Smaller LMR | 4072 | -22 | [-0.05 +/- 3.97](https://gedas.pythonanywhere.com/test/1156/) |  |
| 4.48 | No history bonus in qsearch | 4068 | -4 | [-1.93 +/- 4.08](https://gedas.pythonanywhere.com/test/1174/) | [+0.41 +/- 3.03](https://gedas.pythonanywhere.com/test/1178/) |
| 4.49 | Simplify is_attacked and speedup | 4061 | -7 | [+0.90 +/- 6.25](https://gedas.pythonanywhere.com/test/1182/) |  |
| 4.50 | Exclude pawn control for king attacks | 4058 | -3 | [-0.01 +/- 2.87](https://gedas.pythonanywhere.com/test/1187/) | [+0.78 +/- 3.29](https://gedas.pythonanywhere.com/test/1188/) |
| 4.51 | Endgame scaling by remaining pawn count | 4096 | +38 | [+5.63 +/- 4.21](https://gedas.pythonanywhere.com/test/1211/) | [+18.16 +/- 8.50](https://gedas.pythonanywhere.com/test/1212/) |
| 4.52 | Fix early exit logic | 4096 | = | [+3.05 +/- 4.70](https://gedas.pythonanywhere.com/test/1222/) | [+13.56 +/- 8.14](https://gedas.pythonanywhere.com/test/1223/) |
| 4.53 | Smaller loader | 4083 | -13 |  |  |
| 4.54 | Full ASM loader | 4079 | -4 |  |  |
| 4.55 | Remove max_material | 4076 | -3 |  |  |
| 4.56 | Smaller syscalls | 4069 | -7 |  |  |
| 4.57 | More RFP if improving | 4078 | +9 | [+6.63 +/- 4.68](https://gedas.pythonanywhere.com/test/1253/) | [+6.17 +/- 4.37](https://gedas.pythonanywhere.com/test/1255/) |
| 4.58 | Adjust NMP depth by beta distance to eval | 4095 | +17 | [+7.96 +/- 5.25](https://gedas.pythonanywhere.com/test/1270/) | [+4.24 +/- 3.36](https://gedas.pythonanywhere.com/test/1177/) |
| 4.59 | Aspiration windows; Remove More RFP if improving; Remove Adjust NMP depth by beta distance to eval | 4094 | -1 | [+10.16 +/- 6.17](https://gedas.pythonanywhere.com/test/1291/) | [+13.03 +/- 6.89](https://gedas.pythonanywhere.com/test/1292/) |
| 4.60 | Aspiration window margin 16 | 4094 | = | [+7.66 +/- 5.21](https://gedas.pythonanywhere.com/test/1293/) | [+6.35 +/- 13.86](https://gedas.pythonanywhere.com/test/1297/) |
| 4.61 | More RFP if improving | 4096 | +2 | [+8.44 +/- 5.50](https://gedas.pythonanywhere.com/test/1301/) | [+4.72 +/- 3.60](https://gedas.pythonanywhere.com/test/1303/) |
| 4.62 | NMP base depth reduction 4 | 4096 | = | [+4.89 +/- 3.75](https://gedas.pythonanywhere.com/test/1309/) | [+5.92 +/- 4.23](https://gedas.pythonanywhere.com/test/1310/) |
| 4.63 | More ordering permutations | 4086 | -10 | [+0.99 +/- 3.51](https://gedas.pythonanywhere.com/test/1318/) |  |
| 4.64 | Tune search parameters | 4086 | = | [+11.13 +/- 6.47](https://gedas.pythonanywhere.com/test/1328/) | [+9.56 +/- 5.72](https://gedas.pythonanywhere.com/test/1331/) |
| 4.65 | LMR more for bad histories | 4094 | +8 | [+9.41 +/- 5.83](https://gedas.pythonanywhere.com/test/1342/) | [+5.33 +/- 3.89](https://gedas.pythonanywhere.com/test/1345/) |
| 4.66 | LMR from 3 evaluated moves | 4094 | = | [+5.89 +/- 4.26](https://gedas.pythonanywhere.com/test/1369/) | [+2.70 +/- 2.15](https://gedas.pythonanywhere.com/test/1371/) |
| 4.67 | LMR from depth 2 | 4094 | = | [+9.55 +/- 5.78](https://gedas.pythonanywhere.com/test/1384/) | [+16.04 +/- 7.65](https://gedas.pythonanywhere.com/test/1385/) |
| 5.0 | Equivalent to 4.67 | 4094 | = |  |  |
| 5.1 | Smaller PSTs | 4091 | -3 |  |  |
| 5.2 | Smaller loader (thanks to sqrmax) | 4088 | -3 |  |  |
| 5.3 | LMR from depth 3 | 4088 | = | [-0.00 +/- 2.66](https://gedas.pythonanywhere.com/test/1391/) | [+4.22 +/- 3.28](https://gedas.pythonanywhere.com/test/1393/) |
| 5.4 | No LMR for positive move scores | 4095 | +7 | [+4.17 +/- 3.30](https://gedas.pythonanywhere.com/test/1398/) | [+5.26 +/- 3.85](https://gedas.pythonanywhere.com/test/1402/) |
| 5.5 | No moves check for LMR | 4086 | -9 | [+5.36 +/- 5.50](https://gedas.pythonanywhere.com/test/1406/) | [+4.68 +/- 5.02](https://gedas.pythonanywhere.com/test/1407/) |
| 5.6 | Bit based blackbox | 4084 | -2 |  |  |
| 5.7 | Loader size peephole optimization (implemented by sqrmax) | 4082 | -2 |  |  |
| 5.8 | Protected pawn evaluation | 4095 | +13 | [+20.84 +/- 9.26](https://gedas.pythonanywhere.com/test/1429/) | [+14.03 +/- 7.13](https://gedas.pythonanywhere.com/test/1430/) |
| 5.9 | Smaller flip_pos (idea by sqrmax) | 4087 | -8 | [-0.37 +/- 2.31](https://gedas.pythonanywhere.com/test/1435/) |  |
| 5.10 | Nanosecond time management (idea by sqrmax) | 4077 | -10 | [+8.02 +/- 6.37](https://gedas.pythonanywhere.com/test/1436/) |  |
| 5.11 | TT length power of 2 (idea by sqrmax) | 4072 | -5 | [+0.89 +/- 3.30](https://gedas.pythonanywhere.com/test/1437/) |  |
| 5.12 | King material UB size save | 4069 | -3 | [+0.44 +/- 2.94](https://gedas.pythonanywhere.com/test/1445/) |  |
| 5.13 | Smaller max_time | 4065 | -4 | [+0.76 +/- 3.22](https://gedas.pythonanywhere.com/test/1446/) |  |
| 5.14 | Phalanx pawn evaluation | 4081 | +16 | [+12.08 +/- 6.67](https://gedas.pythonanywhere.com/test/1454/) | [+17.86 +/- 7.98](https://gedas.pythonanywhere.com/test/1456/) |
| 5.15 | Phase UB size save | 4079 | -2 |  |  |
| 5.16 | Reverse loop size save (idea by sqrmax) | 4078 | -1 |  |  |
| 5.17 | Decrease loader size | 4074 | -4 |  |  |
| 5.18 | Decrease loader size | 4072 | -2 |  |  |
| 5.18 | Decrease loader size | 4070 | -2 |  |  |
| 5.19 | Move score pruning | 4086 | +16 | [+14.19 +/- 7.39](https://gedas.pythonanywhere.com/test/1493/) | [+14.06 +/- 6.96](https://gedas.pythonanywhere.com/test/1494/) |
| 5.20 | Decrease loader size | 4085 | -1 |  |  |
| 5.21 | Loader ELF header smashing | 4052 | -33 |  |  |
| 5.22 | ELF / PHDR header overlapping | 4044 | -8 |  |  |
| 5.23 | Disable second instruction scheduling pass | 3994 | -50 | [-1.50 +/- 1.93](https://gedas.pythonanywhere.com/test/1545/) |  |
| 5.24 | Move score pruning margin 128 | 3994 | = | [+8.59 +/- 5.43](https://gedas.pythonanywhere.com/test/1553/) | [+16.93 +/- 7.96](https://gedas.pythonanywhere.com/test/1555/) |
| 5.25 | Unify eval value calculation | 3988 | -6 |  |  |
| 5.26 | Smaller atoi and go | 3982 | -6 |  |  |
| 5.27 | Smaller castling | 3974 | -8 |  |  |
| 5.28 | Smaller diagonal mask compute | 3973 | -1 |  |  |
| 5.29 | Inline comparisons | 3964 | -9 |  |  |
| 5.30 | Reduce size of king and knight | 3961 | -3 | [+2.23 +/- 4.07](https://gedas.pythonanywhere.com/test/1572/) |  |
| 5.31 | Smaller getl | 3958 | -3 |  |  |
| 5.32 | Repetition detection fix | 3960 | +2 | [+0.83 +/- 3.24](https://gedas.pythonanywhere.com/test/1581/) |  |
| 5.33 | Moves not in stack | 3961 | +1 | [-0.50 +/- 2.21](https://gedas.pythonanywhere.com/test/1587/) |  |
| 5.34 | Lazy SMP; Remove full repetition detection | 4079 | +118 | [+2.31 +/- 15.72](https://gedas.pythonanywhere.com/test/1602/) |  |
| 6.0 | Equivalent to 5.34 | 4079 |  |  |  |
| 6.1 | 1MB child thread stack size | 4074 | -5 |  |  |
| 6.2 | Pass thread data to search | 4072 | -2 | [+3.12 +/- 4.54](https://gedas.pythonanywhere.com/test/1610/) |  |
| 6.3 | Exclude unneeded variables for mini | 4070 | -2 |  |  |
| 6.4 | Optimize mobility | 4072 | +2 | [+9.26 +/- 6.80](https://gedas.pythonanywhere.com/test/1612/) |  |
| 6.5 | Tune search parameters | 4077 | +5 | [-0.30 +/- 2.95](https://gedas.pythonanywhere.com/test/1614/) | [+10.21 +/- 5.86](https://gedas.pythonanywhere.com/test/1613/) |
| 6.6 | Extact bbs in eval | 4077 | = |  |  |
| 6.7 | Simplify king atack eval | 4068 | -9 | [+2.04 +/- 3.98](https://gedas.pythonanywhere.com/test/1624/) |  |
| 6.8 | LMR move score divisor 384 | 4055 | -13 | [-0.84 +/- 3.29](https://gedas.pythonanywhere.com/test/1628/) | [+4.00 +/- 3.13](https://gedas.pythonanywhere.com/test/1629/) |
| 6.9 | Full open file evaluation | 4096 | +41 | [+11.84 +/- 6.53](https://gedas.pythonanywhere.com/test/1662/) |  |
| 6.10 | Split thread head and data | 4089 | -5 | [+2.21 +/- 16.74](https://gedas.pythonanywhere.com/test/1664/) |  |
| 7.0 | Use PAQ1-based compression (compressor and loader derived from Crinkler) | 4021 | -68 |  |  |
| 7.1 | Blackbox after PAQ | 3980 | -41 | [-13.98 +/- 25.77](https://gedas.pythonanywhere.com/test/1672/) |  |
| 7.2 | Bits in PAQ header | 3974 | -6 |  |  |
| 7.3 | Remove baseprob from header; Remove hashbits from header; Use hashmask directly in loader | 3962 | -12 |  |  |
| 7.4 | Shift-out-zero to terminate weightmask loop | 3959 | -3 |  |  |
| 7.5 | Run bitwise blackbox | 3952 | -7 |  |  |
| 7.6 | Remove hash reduction | 3940 | -12 |  |  |
| 7.7 | ELF smashing | 3917 | -23 |  |  |
| 7.8 | More ELF smashing | 3910 | -7 |  |  |
| 7.9 | Smaller loader | 3865 | -45 |  |  |
| 7.10 | Smaller loader | 3852 | -13 |  |  |
| 7.11 | No used check in compression | 3848 | -4 |  |  |
| 7.12 | Smaller loader | 3847 | -1 |  |  |
| 7.13 | Use direct syscalls | 3842 | -5 |  |  |
| 7.14 | Smaller loader | 3827 | -15 |  |  |
| 7.14 | Smaller loader | 3821 | -6 |  |  |
| 7.15 | Smaller loader | 3811 | -9 |  |  |
| 7.16 | Rerun blackbox | 3808 | -3 |  |  |
| 7.17 | King shield evaluation | 3860 | +52 | [+14.17 +/- 7.40](https://gedas.pythonanywhere.com/test/1678/) | [+13.46 +/- 6.97](https://gedas.pythonanywhere.com/test/1679/) |
| 7.18 | Pawn threat evaluation | 3885 | +25 | [+9.15 +/- 5.73](https://gedas.pythonanywhere.com/test/1686/) | [+9.21 +/- 5.58](https://gedas.pythonanywhere.com/test/1687/) |
| 7.19 | Bishop colour pawn evaluation | 3930 | +45 | [+10.29 +/- 6.08](https://gedas.pythonanywhere.com/test/1689/) | [+5.07 +/- 3.75](https://gedas.pythonanywhere.com/test/1690/) |
| 7.20 | Reduce size in eval | 3925 | -5 |  |  |
| 7.21 | Pawn correction history | 4058 | +128 | [+4.80 +/- 3.65](https://gedas.pythonanywhere.com/test/1722/) | [+14.96 +/- 7.28](https://gedas.pythonanywhere.com/test/1723/) |
| 7.22 | Replace pawn with material correction | 4083 | +25 | [+12.41 +/- 6.74](https://gedas.pythonanywhere.com/test/1749/) | [+19.90 +/- 8.52](https://gedas.pythonanywhere.com/test/1750/) |
| 7.23 | Reun blackbox from uncompressed | 4079 | -4 |  |  |
| 8.00 | Equivalent to 7.23 |  |  |  |  |
| 8.1 | Allow exact updates for corrhist | 4071 | -8 | [-2.94 +/- 2.34](https://gedas.pythonanywhere.com/test/1798/) | [+1.93 +/- 3.72](https://gedas.pythonanywhere.com/test/1799/) |
| 8.2 | SPSA tune | 4073 | +2 | [+10.49 +/- 6.09](https://gedas.pythonanywhere.com/test/1802/) | [+5.54 +/- 4.00](https://gedas.pythonanywhere.com/test/1805/) |
| 8.3 | Corrhist simpler hashing | 4053 | -20 | [+2.04 +/- 3.95](https://gedas.pythonanywhere.com/test/1811/) | [+3.85 +/- 4.63](https://gedas.pythonanywhere.com/test/1812/) |
| 8.4 | 22GB loader memsize | 4053 | = |  |  |
| 8.5 | No thread state set | 4042 | -11 |  |  |
| 8.6 | Pawn correction history | 4090 | +48 | [+17.77 +/- 8.33](https://gedas.pythonanywhere.com/test/1818/) | [+15.07 +/- 7.46](https://gedas.pythonanywhere.com/test/1819/) |
