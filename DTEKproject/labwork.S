  # labwork.S
  # Written by Ludvig Edvinson, 2021
  # Skeleton file for IS1200/IS1500 lab 1.
  # The macros PUSH and POP are in the public domain.
  # Please add your own code at the end of the file.

  #
  # Please keep the two macros PUSH and POP unchanged
  #
.macro	PUSH reg
	addi	$sp,$sp,-4
	sw	\reg,0($sp)
.endm

.macro	POP reg
	lw	\reg,0($sp)
	addi	$sp,$sp,4
.endm
  #
  # Please add your own code below this line
  #
.global delay
.global time2string
.global enable_interrupt
.data
.text

enable_interrupt:
	ei
	jr	$ra
	nop

hexasc:
	andi	$a1,$a1,15
	slti	$t0,$a0,10
	beq	$t0,$0,number
	nop
	addi	$a1,$a1,48
	jr	$ra
	nop
number:
	addi	$a1,$a1,55
	jr	$ra
	nop
	
delay:
	li	$s0,5000
dloop:
	beq	$a0,$0,stop	

	addi	$s0,$s0,-1
	bne	$s0,$0,dloop
	nop
	addi	$a0,$a0,-1
	j	delay
	nop

stop:

	jr	$ra
	nop
	
time2string:
	PUSH $ra
	PUSH $s0 
	
	andi	$s0,$s0,0
	andi	$s1,$s1,0
	add	$s0,$s0,$a1
	
	srl	$a1,$a1,12
	jal	hexasc
	nop
	sb	$a1,0($a0)
	
	srl	$a1,$s0,8
	jal	hexasc
	nop
	sb	$a1,1($a0)	
	
	li	$a1,58
	sb	$a1,2($a0)
	
	srl	$a1,$s0,4
	jal	hexasc
	nop
	sb	$a1,3($a0)
	
	srl	$a1,$s0,0
	jal	hexasc
	nop
	sb	$a1,4($a0)
	
	andi	$a1,$a1,0
	sb	$a1,5($a0)
	
	POP $s0 
	POP $ra
	
	jr	$ra
	nop
	

