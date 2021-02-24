  # labwork.S
  # Written 2015-2017 by F Lundevall
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
  .data
  .text

hexasc:		
	PUSH $s0		# letter check
	andi	$s0,$s0,0x00000000
	
	andi	$a0,$a0,0x0000000f
	slti 	$s0,$a0,10	#converter
	beq	$s0,$0,letter
	nop
	addi	$v0,$a0,48
	POP $s0
	jr	$ra
	nop
	
letter:
	addi	$v0,$a0,55	#letter branch
	POP $s0
	jr	$ra
	nop
	
delay:
	PUSH $s0
	PUSH $s1
	andi	$s0,$s0,0x00000000 
	andi	$s1,$s1,0x00000000	
	
	add	$s0,$s0,$a0
	andi	$a0,$a0,0x00000000
	addi	$a0,$a0,1000
	addi	$s1,$s1,4500	# delay counter
	mul	$s1,$s1,$a0
	andi	$a0,$a0,0x00000000
delayloop1:
	beq	$s0,$0,delaydone
	nop
	addi	$s0,$s0,-1
	j	delayloop2
	nop
	
delaydone:
	POP $s1
	POP $s0
 	jr $ra
	nop
	
delayloop2:
	beq	$s1,$0,delayloop1
	nop
	addi	$s1,$s1,-1
	j	delayloop2
	nop

	
time2string:
	PUSH $ra
	PUSH $s0 		# timestr adress
	PUSH $s1 		# time-info
	
	andi	$s0,$s0,0x00000000 
	andi	$s1,$s1,0x00000000
	
	move 	$s0,$a0 	# save adress
	add 	$s1,$0,$a1 	# save info
	
	andi	$a0,$s1,0x0000f000
	srl	$a0,$s1,12	# get first number
	jal	hexasc		#
	nop
	sb 	$v0,0($s0)	# add storeage of char 1 to string
	
	andi	$a0,$s1,0x00000f00
	srl	$a0,$s1,8
	jal	hexasc		#
	nop
	sb 	$v0,1($s0)	# add storeage of char 2 to string
	
	andi	$a0,$a0,0x00000000
	addi	$a0,$a0,58
	sb 	$a0,2($s0)	# add ":" to storage (0x3a / 58)
	
	andi	$a0,$a0,0x00000000
	andi	$a0,$s1,0x000000f0
	srl	$a0,$s1,4
	jal	hexasc		#
	nop
	sb 	$v0,3($s0)	# add storeage of char 3 to string
	
	andi	$a0,$s1,0x0000000f
	jal	hexasc		#
	nop	
	sb 	$v0,4($s0)	# add storeage of char 4 to string
	
	andi	$a0,$a0,0x00000000
	sb 	$a0,5($s0)	#add "null" to storage (0)
	
	andi	$a0,$a0,0x00000000
	
	
	POP $s1
	POP $s0
	POP $ra
	
	jr	$ra
	nop
