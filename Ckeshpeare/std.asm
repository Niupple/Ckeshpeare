.data
output: .asciiz "5 != 120\nx = 5\ny = 10\nSWAP x = 10\nSWAP y = 5\ncomplete number: 6\n  1\n  2\n  3\n \ncomplete number: 28\n  1\n  2\n  4\n  7\n  14\n \n---------------------------------------------------------------\n'water flower'number is:\n  153\n \n---------------------------------------------------------------\n 2\n 3\n 5\n 7\n 11\n 13\n 17\n 19\n 23\n 29\n \n 31\n 37\n 41\n 43\n 47\n 53\n 59\n 61\n 67\n 71\n \n 73\n 79\n 83\n 89\n 97\n 101\n 103\n 107\n 109\n 113\n \n 127\nThe total is 31\n\n"

.text


li $t0, 13000

loop1:
addu $t0, $t0, $0
addu $t0, $t0, $0
addu $t0, $t0, $0
addiu $t0, $t0, -1
bgez $t0, loop1

li $t0, 11950
loop2:
addu $t0, $t0, $0
addu $t0, $t0, $0
addu $t0, $t0, $0
addiu $t0, $t0, -1
j nothing
nothing:
bgez $t0, loop2

li $t0, 390
loop3:
sw $t0, 0($sp)
addiu $t0, $t0, -1
bgez $t0, loop3

li $v0 41

li $t0, 160
loop4:
sw $t0, 0($sp)
addiu $t0, $t0, -1
syscall
bgez $t0, loop4

li $v0, 4
la $a0, output
syscall
