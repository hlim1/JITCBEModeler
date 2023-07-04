#!/usr/bin/ruby -w

#currently the FS register isn't being handled correctly, the matching PIN reg is REG_SEG_FS_BASE, but we want to transform both REG_SEG_FS and REG_SEG_FS_BASE to the same thing

INVALID_REG = {:NAME => "INVALID", :ARCH => [:MIC,:IA32,:IA32E], :SIZE64 => 0, :SIZE32 => 0, :OFFSET => 0}
INVALID_REG[:FULL] = {:MIC => INVALID_REG, :IA32 => INVALID_REG, :IA32E => INVALID_REG}

ARCHS = [:IA32, :IA32E, :MIC, :NONE]

def checkRegArch(reg)
	if(reg[:ARCH].empty? || reg[:ARCH].inject(false) {|sum,arch| sum || !ARCHS.include?(arch)})
		puts("Unknown Arch on line: #{line}")
		exit(1)
	end
end

def readRegs(file)
	lineNum = 1
	line = file.gets
	regs = []
	groups = {}
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
		reg = {:NAME => $2, :SIZE64 => $3, :SIZE32 => $5 ? $5 : $3, :OFFSET => 0, :PARTIALS => [], :FULL => nil, :GROUP => $6.to_sym,  :ARCH => ($1 == "ALL") ? [:MIC,:IA32,:IA32E] : $1.split(',').map {|x| x.to_sym}}
	
		checkRegArch(reg)
		
		groups[reg[:GROUP]] = [] if groups[reg[:GROUP]] == nil
		groups[reg[:GROUP]].push(reg)

		line = file.gets
		lineNum += 1
		while(line =~ /^\s+([a-zA-Z0-9,_]+)\s*:\s*(\w+)\s+(\d+)\+?(\d+)?\s+([a-zA-Z0-9,_]+)\s*$/)
			if line[0] == '#' || line == ""
				line = file.gets
				lineNum += 1
				next
			end
		
			partialReg = {:NAME => $2, :SIZE64 => $3, :SIZE32 => $3, :OFFSET => $4 ? $4 : 0, :FULL => nil, :GROUP => $5.to_sym, :ARCH => ($1 == "ALL") ? [:MIC,:IA32,:IA32E] : $1.split(',').map {|x| x.to_sym}}

			checkRegArch(partialReg)
			reg[:PARTIALS].push(partialReg)
			
			groups[partialReg[:GROUP]] = [] if groups[partialReg[:GROUP]] == nil
			groups[partialReg[:GROUP]].push(partialReg)
			
			line = file.gets
			lineNum += 1
		end
		
		regs.push(reg)
	end
	
	for reg in regs
		fullRegs = INVALID_REG[:FULL].clone
		reg[:ARCH].each {|arch| fullRegs[arch] = reg}
		reg[:FULL] = fullRegs.clone
		for pReg in reg[:PARTIALS]
			pReg[:ARCH].each {|arch| fullRegs[arch] = pReg if fullRegs[arch] == INVALID_REG }
			pReg[:FULL] = fullRegs.clone
		end
	end
	
	return groups
end

def genEnum(headerFile, groups, orderFile)
	headerFile.puts()
    headerFile.puts("/* This enum represents all of the registers in x86 for all of the different")
    headerFile.puts("   architectures. Note that different architectures use different subsets of")
    headerFile.puts("   these registers and as a result, each of the register groups are named.")
    headerFile.puts("   There should never be an architecture that uses a subset of one of the")
    headerFile.puts("   groups in this file. Also, it should be noted that LynxReg is ordered so")
    headerFile.puts("   that full registers appear first, making it possibly to easily loop over")
    headerFile.puts("   the full registers*/")
	headerFile.puts("typedef enum {")
	regsInOrder = []
	orderGroups = []
	enumVal = "=0"
	orderFile.each_line { |line|
		line.strip!
		
		if(line.empty? || line[0,1] == '#')
			next
		elsif(line =~ /^([a-zA-Z0-9_]+)$/)
			headerFile.puts("\t#{$1}#{enumVal},")
			enumVal = "=#{$1}"
		elsif(line =~ /^\-([a-zA-Z0-9_]+)$/)
			headerFile.puts("\t#{$1}=LYNX_#{regsInOrder[-1][:NAME]},")
		elsif(line =~ /^\[([a-zA-Z0-9_]+)\]$/)
			group = $1.to_sym
			if(!groups[group])
				puts("Unknown group #{group} in orderFile")
				exit(1)
			end
			orderGroups.push(group)
			headerFile.puts("\tLYNX_#{group}_FIRST#{enumVal},")
			enumVal="=LYNX_#{group}_FIRST"
			for reg in groups[group]
				regsInOrder.push(reg)
				headerFile.puts("\tLYNX_#{reg[:NAME]}#{enumVal},")
				enumVal=""
			end
			headerFile.puts("\tLYNX_#{group}_LAST=LYNX_#{regsInOrder[-1][:NAME]},")
		else
			puts("Error: Unable to recognize line #{line}")
			exit(1)
		end
	}

	headerFile.puts("\tLYNX_LAST_REG=LYNX_#{regsInOrder[-1][:NAME]},")
	headerFile.puts("\tLYNX_INVALID,")
	headerFile.puts("\tLYNX_LAST=LYNX_INVALID")
	headerFile.puts("} LynxReg;")

	groups.each_key {|key|
		if(!orderGroups.find_index(key))
			puts("Warning: Missing Group #{key}")
		end
	}
	
	regsInOrder.push(INVALID_REG)
	return regsInOrder
end

def genLynxReg2Str(cFile, headerFile, regsInOrder) 
	cFile.puts("const char *LynxReg2StrArr[] = {")
	
	for reg in regsInOrder
		cFile.puts("\t\"#{reg[:NAME]}\",")
	end

	cFile.puts("};")

    headerFile.puts()
	headerFile.puts("//Gets a string representation of the given LynxReg")
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
    headerFile.puts()
	headerFile.puts("//Gets the size of the lReg in a 64 bit architecture")
	headerFile.puts("uint8_t LynxRegSize(LynxReg lReg);")
    headerFile.puts()
	headerFile.puts("//Gets the size of the lReg in a 32 bit architecture")
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

    headerFile.puts()	
	headerFile.puts("//Gets the number of bytes this register is offset from the least significant bit of the fullReg")
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

    headerFile.puts()
	headerFile.puts("//Gets the full register for the input register if all registers are considered")
	headerFile.puts("LynxReg LynxReg2FullLynxReg(LynxReg lReg);")
    headerFile.puts()
	headerFile.puts("//Gets the full register for the input register if 32bit registers are considered")
	headerFile.puts("LynxReg LynxReg2FullLynxIA32EReg(LynxReg lReg);")
    headerFile.puts()
	headerFile.puts("//Gets the full register for the input register if 64bit registers are considered (Excluding MIC, which is the ZMM registers)")
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
    headerFile.puts()
	headerFile.puts("//Gets the LynxReg that corresponds to the given PIN reg")
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

    headerFile.puts()
	headerFile.puts("//Gets the PIN REG that corresponds to the given LynxReg")
	headerFile.puts("REG LynxReg2Reg(LynxReg lReg);")

	cppFile.puts("REG LynxReg2Reg(LynxReg lReg) {")
	cppFile.puts("\treturn LynxReg2RegArr[lReg];")
	cppFile.puts("}")
end

#just in case, I'm keeping this around
# def genLynxReg2CapstoneReg(cFile, headerFile, regsInOrder)
	# headerFile.puts("LynxReg Reg2LynxReg(x86_reg reg);")

	# cFile.puts("LynxReg Reg2LynxReg(x86_reg reg) {")
	# cFile.puts("\tswitch(reg) {")
	
	# for reg in regsInOrder
		# ind = reg[:NAME].index("_")
		# cFile.puts("\t\tcase X86_REG_#{reg[:NAME][(ind ? ind+1 : 0) .. -1]}:")
		# cFile.puts("\t\t\treturn LYNX_#{reg[:NAME]};")
	# end
	# cFile.puts("\t\tdefault:")
	# cFile.puts("\t\t\tfprintf(stderr, \"Unknown Reg: %d\\n\", reg);")
	# cFile.puts("\t}")
	# cFile.puts("\treturn LYNX_INVALID;")
	# cFile.puts("}")
# end

if(ARGV.size != 2)
	puts("Usage: gen.rb regsFile orderFile")
	exit(1)
end


regFile = File.new(ARGV[0], "r")
orderFile = File.new(ARGV[1], "r")

groupedRegs = readRegs(regFile)

headerFile = File.new("LynxReg.h", "w")
cFile = File.new("LynxReg.c", "w")

headerFile.puts('#ifndef _LYNX_REG_H_')
headerFile.puts('#define _LYNX_REG_H_')
headerFile.puts('#include <stdint.h>')

cFile.puts('#include "LynxReg.h"')
cFile.puts()

regsInOrder = genEnum(headerFile, groupedRegs, orderFile)
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

puts("There are #{regsInOrder.length} registers")
