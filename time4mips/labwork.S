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
	PUSH $s5 		# letter check
	
	andi	$a3,$a3,0x0000000f
	slti 	$s5,$a3,10	#converter
	beq	$a1,$0,letter
	nop
	addi	$v0,$a3,48
	POP $s5
	jr	$ra
	nop
	
letter:
	addi	$v0,$a3,55	#letter branch
	POP $s5
	jr	$ra
	nop
	
delay:
	PUSH $s6
	PUSH $s7
	add	$s6,$s6,$a0
	addi	$s7,$s7,4711	# delay counter
delayloop1:
	beq	$s6,$0,delaydone
	nop
	addi	$s6,$s6,-1
	j	delayloop2
	nop
	
delaydone:
	POP	$s7
	POP	$s6
 	jr $ra
	nop
	
delayloop2:
	beq	$s7,$0,delayloop1
	nop
	addi	$s7,$s7,-1
	j	delayloop2
	nop

	
time2string:
	PUSH $ra
	PUSH $s0		# timestr adress
	PUSH $s1		# time-info
	PUSH $s2		# temporary shift storage
	PUSH $s3		# prev deleter
	PUSH $s4		# used for misc ascii chars
	
	move 	$s0,$a0 	# save adress
	add 	$s1,$0,$a1 	# save info
	
	srl	$s3,$a1,16
	sll	$s3,$s3,16
	xor	$s1,$s1,$s3
	xor	$s3,$s3,$s3
	
	srl	$a3,$s1,12	# get first number
	jal	hexasc		#
	nop
	sll	$s3,$a3,12	#
	xor	$s1,$s1,$s3	# remove first number from time-info
	sb 	$v0,0($s0)	# add storeage of char 1 to string
	
	srl	$a3,$s1,8	# get second number
	jal	hexasc		#
	nop
	sll	$s3,$a3,8	#
	xor	$s1,$s1,$s3	# remove first number from time-info
	sb 	$v0,1($s0)	# add storeage of char 2 to string
	
	addi	$s4,$s4,58
	sb 	$s4,2($s0)	# add ":" to storage (0x3a / 58)
	
	srl	$a3,$s1,4	# get third number
	jal	hexasc		#
	nop
	sll	$s3,$a3,4	#
	xor	$s1,$s1,$s3	# remove first number from time-info
	sb 	$v0,3($s0)	# add storeage of char 3 to string
	
	srl	$a3,$s1,0	# get last number
	jal	hexasc		#
	nop	
	sb 	$v0,4($s0)	# add storeage of char 4 to string
	
	addi	$s4,$s4,-58
	sb 	$s4,5($s0)	#add "null" to storage (0)
	
	andi	$a3,$0,0x00000000
	andi	$a0,$a0,0x00000000
			
	li	$v0,11		# printz
	syscall
	
	
	POP $s4
	POP $s3
	POP $s2
	POP $s1
	POP $s0
	POP $ra
	
	jr	$ra
	nop
