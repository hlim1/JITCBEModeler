#!/usr/bin/ruby -w

INVALID_REG = {:NAME => "INVALID", :ARCH => [:MIC,:IA32,:IA32E], :SIZE64 => 0, :SIZE32 => 0, :OFFSET => 0}
INVALID_REG[:FULL] = {:MIC => INVALID_REG, :IA32 => INVALID_REG, :IA32E => INVALID_REG}

ARCHS = [:IA32, :IA32E, :MIC]

def checkRegArch(reg)
	if(reg[:ARCH].empty? || reg[:ARCH].inject(false) {|sum,arch| sum || !ARCHS.include?(arch)})
		puts("Unknown Arch on line: #{line}")
		exit(1)
	end
end

def read(file, regs)
	lineNum = 1
	line = file.gets
	while(line)
		
		if line.strip == "" || line[0,1] == '#'
			line = file.gets
			lineNum += 1
			next
		end
		
#		if(!(line =~ /^(?<name>\w+)\s+(?<size>\d+)\+?(?<offset>\d+)?\s+(?<grp>\w+)\s*$/))
		if(!(line =~ /^([a-zA-Z0-9,_]+)\s*:\s*(\w+)\s+(\d+)(\((\d+)\))?\s+([a-zA-Z0-9,_]+)\s*$/))
			puts("Error on line #{lineNum}: #{line}")
			p regs
			exit(1)
		end
		reg = {:NAME => $2, :SIZE64 => $3, :SIZE32 => $5 ? $5 : $3, :OFFSET => 0, :PARTIALS => [], :FULL => INVALID_REG[:FULL].clone, :GROUP => $6,  :ARCH => ($1 == "ALL") ? [:MIC,:IA32,:IA32E] : $1.split(',').map {|x| x.to_sym}}
	
		checkRegArch(reg)

		line = file.gets
		lineNum += 1
		while(line =~ /^\s+([a-zA-Z0-9,_]+)\s*:\s*(\w+)\s+(\d+)\+?(\d+)?\s+([a-zA-Z0-9,_]+)\s*$/)
			if line[0] == '#' || line == ""
				line = file.gets
				lineNum += 1
				next
			end
		
			partialReg = {:NAME => $2, :SIZE64 => $3, :SIZE32 => $3, :OFFSET => $4 ? $4 : 0, :FULL => INVALID_REG[:FULL].clone, :GROUP => $5, :ARCH => ($1 == "ALL") ? [:MIC,:IA32,:IA32E] : $1.split(',').map {|x| x.to_sym}}

			checkRegArch(partialReg)
			reg[:PARTIALS].push(partialReg)
			line = file.gets
			lineNum += 1
		end
		
		regs.push(reg)
	end
end

def genEnum(headerFile, regs)
	headerFile.puts()
	headerFile.puts("typedef enum {")
	headerFile.puts("\tLYNX_FIRST=0,")
	headerFile.puts("\tLYNX_x64_FIRST=LYNX_FIRST,")

	regsInOrder = []
#	invalidReg = {:NAME => "INVALID", :ARCH => nil, :SIZE64 => 0, :SIZE32 => 0, :OFFSET => 0}
#	invalidReg[:FULL] = {:MIC => invalidReg, :IA32 => invalidReg, :IA32E => invalidReg}

	#print x64 regs first
	enumVal = "=LYNX_x64_FIRST"
	for reg in regs
		if(!reg[:ARCH].include?(:IA32))
			regsInOrder.push(reg)
			headerFile.puts("\tLYNX_#{reg[:NAME]}#{enumVal},")
			reg[:ARCH].each{|arch| reg[:FULL][arch] = reg }
			enumVal = ""
		end
	end

	headerFile.puts("\tLYNX_x86_FIRST,")

	#print remaining full registers in all architectures next
	enumVal = "=LYNX_x86_FIRST"
	for reg in regs
		if(reg[:ARCH].include?(:IA32))
			regsInOrder.push(reg)
			headerFile.puts("\tLYNX_#{reg[:NAME]}#{enumVal},")
			enumVal = ""
			reg[:ARCH].each{|arch| reg[:FULL][arch] = reg }
		end
	end

	headerFile.puts("\tLYNX_x64_LAST=LYNX_#{regsInOrder[-1][:NAME]},")

#	for reg in regs
#		if(reg[:ARCH] == "x86")
#			regsInOrder.push(reg)
#			headerFile.puts("\tLYNX_#{reg[:NAME]}#{enumVal},")
#			enumVal = ""
#			reg[:FULL64] = invalidReg
#			reg[:FULL32] = reg
#		end
#	end

	#print out any registers that are full in x86 but not x86-64 first
	for reg in regs
		if(!reg[:ARCH].include?(:IA32))
			for pReg in reg[:PARTIALS]
				if(pReg[:ARCH].include?(:IA32))
                	        	headerFile.puts("\tLYNX_#{pReg[:NAME]},")
					regsInOrder.push(pReg)
				
					pReg[:ARCH].each {|arch| pReg[:FULL][arch] = pReg}
					reg[:ARCH].each {|arch| pReg[:FULL][arch] = reg}
					break
				end
                	end
		end
        end

	headerFile.puts("\tLYNX_x86_LAST=LYNX_#{regsInOrder[-1][:NAME]},")

	#all remaining partial regs
	for reg in regs
		fullRegs = INVALID_REG[:FULL].clone
		reg[:ARCH].each {|arch| fullRegs[arch] = reg}
		for pReg in reg[:PARTIALS]
			pReg[:ARCH].each {|arch| fullRegs[arch] = pReg if fullRegs[arch] == INVALID_REG }

			if(!regsInOrder.include?(pReg))
                        	headerFile.puts("\tLYNX_#{pReg[:NAME]},")
				regsInOrder.push(pReg)
				pReg[:ARCH].each {|arch| pReg[:FULL][arch] = fullRegs[arch]}
			end


#			if(reg32 == invalidReg)
#				reg32 = pReg if pReg[:ARCH] != "x64"
#			end
#			if(pReg[:ARCH] == nil)
 #                       	headerFile.puts("\tLYNX_#{pReg[:NAME]},")
#				regsInOrder.push(pReg)
#				pReg[:FULL64] = reg64
#				pReg[:FULL32] = reg32
#			end
                end
        end

	headerFile.puts("\tLYNX_LAST_REG=LYNX_#{regsInOrder[-1][:NAME]},")
	headerFile.puts("\tLYNX_INVALID,")
	headerFile.puts("\tLYNX_LAST=LYNX_INVALID")
	headerFile.puts("} LynxReg;")

	regsInOrder.push(INVALID_REG)
	return regsInOrder
end


#def printReg2LynxReg(file, allRegs) 
#	file.puts("LynxReg Reg2LynxReg(REG reg) {")
#	file.puts("\tswitch(reg) {")
#	
#	printEnd = false
#	for entry in allRegs
#		if(entry[:CONDITION][0..4] != "#elif" && printEnd)
#			file.puts("#endif") 
#			printEnd = false
#		end     
#		file.puts(entry[:CONDITION]) if entry[:CONDITION] != ""
#		printEnd = true if entry[:CONDITION][0..2] == "#if"
#		for reg in entry[:REGS]
#			file.puts("\t\tcase REG_#{reg}:")
#			file.puts("\t\t\treturn LYNX_#{reg};")
#		end 
#	end
#	file.puts("\t\tdefault:")
#	file.puts("\t\t\tPIN_MutexLock(&fileWriteLock);")
#	file.puts("\t\t\tfprintf(errorFile, \"Unknown Reg: %s\\n\", REG_StringShort(reg));")
#	file.puts("\t\t\tPIN_MutexUnlock(&fileWriteLock);")
#	file.puts("\t\t\treturn LYNX_INVALID;")
#	file.puts("\t}")
#	file.puts("}")
#end

def genLynxReg2Str(cFile, headerFile, regsInOrder) 
	cFile.puts("const char *LynxReg2StrArr[] = {")
	
	for reg in regsInOrder
		cFile.puts("\t\"#{reg[:NAME]}\",")
	end

	cFile.puts("};")

	headerFile.puts("const char *LynxReg2Str(LynxReg lReg);")
	cFile.puts("const char *LynxReg2Str(LynxReg lReg) {")
	cFile.puts("\treturn LynxReg2StrArr[lReg];")
	cFile.puts("}")
end

def genLynxRegSize(cFile, headerFile, regsInOrder) 
	cFile.puts("const uint8_t LynxRegSizeArr[] = {")
	
	for reg in regsInOrder
		cFile.puts("\t#{reg[:SIZE64]},")
	end
	
	cFile.puts("};")

	cFile.puts("const uint8_t LynxRegSize32Arr[] = {")

	for reg in regsInOrder
		cFile.puts("\t#{reg[:SIZE32]},")
	end

	cFile.puts("};")
	
	headerFile.puts("uint8_t LynxRegSize(LynxReg lReg);")
	headerFile.puts("uint8_t LynxRegSize32(LynxReg lReg);")

	cFile.puts("uint8_t LynxRegSize(LynxReg lReg) {")
	cFile.puts("\treturn LynxRegSizeArr[lReg];")
	cFile.puts("}")
	
	cFile.puts("uint8_t LynxRegSize32(LynxReg lReg) {")
	cFile.puts("\treturn LynxRegSize32Arr[lReg];")
	cFile.puts("}")
end

def genLynxRegOffset(cFile, headerFile, regsInOrder) 
	cFile.puts("const uint8_t LynxRegOffsetArr[] = {")
	
	for reg in regsInOrder
		cFile.puts("\t#{reg[:OFFSET]},")
	end
	
	cFile.puts("};")
	
	headerFile.puts("uint8_t LynxRegOffset(LynxReg lReg);")
	cFile.puts("uint8_t LynxRegOffset(LynxReg lReg) {")
	cFile.puts("\treturn LynxRegOffsetArr[lReg];")
	cFile.puts("}")
end

def genLynxReg2FullLynxReg(cFile, headerFile, regsInOrder)
	cFile.puts("const LynxReg LynxReg2FullLynxRegArr[] = {")

	for reg in regsInOrder
		cFile.puts("\tLYNX_#{reg[:FULL][:MIC][:NAME]},")
	end

	cFile.puts("};")

	cFile.puts("const LynxReg LynxReg2FullLynxIA32ERegArr[] = {")
	for reg in regsInOrder
		cFile.puts("\tLYNX_#{reg[:FULL][:IA32E][:NAME]},")
	end

	cFile.puts("};")

	cFile.puts("const LynxReg LynxReg2FullLynxIA32RegArr[] = {")

	for reg in regsInOrder
		cFile.puts("\tLYNX_#{reg[:FULL][:IA32][:NAME]},")
	end

	cFile.puts("};")

	headerFile.puts("LynxReg LynxReg2FullLynxReg(LynxReg lReg);")
	headerFile.puts("LynxReg LynxReg2FullLynxIA32EReg(LynxReg lReg);")
	headerFile.puts("LynxReg LynxReg2FullLynxIA32Reg(LynxReg lReg);")

	cFile.puts("LynxReg LynxReg2FullLynxReg(LynxReg lReg) {")
	cFile.puts("\treturn LynxReg2FullLynxRegArr[lReg];")
	cFile.puts("}")

	cFile.puts("LynxReg LynxReg2FullLynxIA32EReg(LynxReg lReg) {")
	cFile.puts("\treturn LynxReg2FullLynxIA32ERegArr[lReg];")
	cFile.puts("}")

	cFile.puts("LynxReg LynxReg2FullLynxIA32Reg(LynxReg lReg) {")
	cFile.puts("\treturn LynxReg2FullLynxIA32RegArr[lReg];")
	cFile.puts("}")
end

def genLynxReg2PinReg(cppFile, headerFile, regsInOrder)
	headerFile.puts("LynxReg Reg2LynxReg(REG reg);")

	cppFile.puts("LynxReg Reg2LynxReg(REG reg) {")
	cppFile.puts("\tswitch(reg) {")
	
	for arch in ARCHS
		cppFile.puts("#ifdef TARGET_#{arch.to_s}")
		for reg in regsInOrder
			if(reg[:ARCH].include?(arch))
				cppFile.puts("\t\tcase REG_#{reg[:NAME]}#{"_" if reg[:NAME] == "INVALID"}:")
				cppFile.puts("\t\t\treturn LYNX_#{reg[:NAME]};")
			end
		end
		cppFile.puts("#endif")
	end
	cppFile.puts("\t\tdefault:")
	cppFile.puts("\t\t\tPIN_MutexLock(&errorLock);")
	cppFile.puts("\t\t\tfprintf(errorFile, \"Unknown Reg: %d\\n\", reg);")
	cppFile.puts("\t\t\tPIN_MutexUnlock(&errorLock);")
	cppFile.puts("\t}")
	cppFile.puts("\treturn LYNX_INVALID;")
	cppFile.puts("}")
end

def genPinReg2LynxReg(cppFile, headerFile, regsInOrder)
#	headerFile.puts("REG LynxReg2Reg(LynxReg lReg);")
	cppFile.puts("const REG LynxReg2RegArr[] = {")

	for arch in ARCHS
		cppFile.puts("#ifdef TARGET_#{arch.to_s}")
		for reg in regsInOrder
			if(reg[:ARCH].include?(arch))
				cppFile.puts("\tREG_#{reg[:NAME]}#{"_" if reg[:NAME] == "INVALID"},")
			else
				cppFile.puts("\tREG_INVALID_,")
			end
		end
		cppFile.puts("#endif")
	end
	cppFile.puts("};")

	headerFile.puts("REG LynxReg2Reg(LynxReg lReg);")

	cppFile.puts("REG LynxReg2Reg(LynxReg lReg) {")
	cppFile.puts("\treturn LynxReg2RegArr[lReg];")
	cppFile.puts("}")
end

def genLynxReg2CapstoneReg(cFile, headerFile, regsInOrder)
	headerFile.puts("LynxReg Reg2LynxReg(x86_reg reg);")

	cFile.puts("LynxReg Reg2LynxReg(x86_reg reg) {")
	cFile.puts("\tswitch(reg) {")
	
	for reg in regsInOrder
		ind = reg[:NAME].index("_")
		cFile.puts("\t\tcase X86_REG_#{reg[:NAME][(ind ? ind+1 : 0) .. -1]}:")
		cFile.puts("\t\t\treturn LYNX_#{reg[:NAME]};")
	end
	cFile.puts("\t\tdefault:")
	cFile.puts("\t\t\tfprintf(stderr, \"Unknown Reg: %d\\n\", reg);")
	cFile.puts("\t}")
	cFile.puts("\treturn LYNX_INVALID;")
	cFile.puts("}")
end

if(ARGV.size != 1)
	puts("Usage: gen.rb regsFile")
	exit(1)
end


regFile = File.new(ARGV[0], "r")
regs = []
read(regFile, regs)

headerFile = File.new("LynxReg.h", "w")
cFile = File.new("LynxReg.c", "w")

headerFile.puts('#ifndef _LYNX_REG_H_')
headerFile.puts('#define _LYNX_REG_H_')
headerFile.puts('#include <stdint.h>')

cFile.puts('#include "LynxReg.h"')
cFile.puts()

regsInOrder = genEnum(headerFile, regs)
genLynxReg2Str(cFile, headerFile, regsInOrder)
genLynxRegSize(cFile, headerFile, regsInOrder)
genLynxRegOffset(cFile, headerFile, regsInOrder)
genLynxReg2FullLynxReg(cFile, headerFile, regsInOrder)

headerFile.puts("#endif")

#write pin stuff
pinHeaderFile = File.new("PinLynxReg.h", "w")
pinCppFile = File.new("PinLynxReg.cpp", "w")

pinHeaderFile.puts("#ifndef _PIN_LYNX_REG_H_")
pinHeaderFile.puts("#define _PIN_LYNX_REG_H_")
pinHeaderFile.puts('#include "pin.H"')
pinHeaderFile.puts('#include <cstdio>')
pinHeaderFile.puts("extern \"C\" {")
pinHeaderFile.puts('#include "../shared/LynxReg.h"')
pinHeaderFile.puts("}")

pinCppFile.puts('#include "PinLynxReg.h"')
pinCppFile.puts()

pinHeaderFile.puts("extern PIN_MUTEX errorLock;")
pinHeaderFile.puts("extern FILE *errorFile;")

genPinReg2LynxReg(pinCppFile, pinHeaderFile, regsInOrder)
genLynxReg2PinReg(pinCppFile, pinHeaderFile, regsInOrder)

pinHeaderFile.puts("#endif")

#write capstone stuff
csHeaderFile = File.new("CapstoneLynxReg.h", "w")
csCFile = File.new("CapstoneLynxReg.c", "w")

csHeaderFile.puts("#ifndef _CAPSTONE_LYNX_REG_H_")
csHeaderFile.puts("#define _CAPSTONE_LYNX_REG_H_")
csHeaderFile.puts('#include "capstone.h"')
csHeaderFile.puts('#include "LynxReg.h"')

csCFile.puts('#include "CapstoneLynxReg.h"')
csCFile.puts("#include <stdio.h>")
csCFile.puts()

genLynxReg2CapstoneReg(csCFile, csHeaderFile, regsInOrder)

csHeaderFile.puts("#endif")

puts("There are #{regsInOrder.length} registers")
