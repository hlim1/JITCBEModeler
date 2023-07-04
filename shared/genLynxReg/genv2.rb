#!/usr/bin/ruby -w
def read(file, regs)
	line = file.gets
	while(line)
		line.strip!
		if line[0] == "#" || line == ""
			line = file.gets
			next
		end
		
#		if(!(line =~ /^(?<name>\w+)\s+(?<size>\d+)\+?(?<offset>\d+)?\s*$/))
		if(!(line =~ /^(?<name>\w+)\s+(?<size>\d+)\s*$/))
			puts("Error: Invalid Line: #{line}")
			exit(1)
		end
		
		reg = {:NAME => $1, :SIZE => $2, :OFFSET => 0, :PARTIALS => []}
		
		line = file.gets
		while(line =~ /^\s+(?<name>\w+)\s+(?<size>\d+)\+?(?<offset>\d+)?\s*$/)
			if line[0] == "#" || line == ""
				line = file.gets
				next
			end
		
			partialReg = {:NAME => $1, :SIZE => $2, :OFFSET => $3 ? $3 : 0 }
			reg[:PARTIALS].push(partialReg)
			line = file.gets
		end
		
		regs.push(reg)
	end
end

def genEnum(headerFile, regs)
	headerFile.puts()
	headerFile.puts("typedef enum {")
	headerFile.puts("\tLYNX_FIRST=0,")
	headerFile.puts("\tLYNX_#{regs[0][:NAME]}=LYNX_FIRST,")
	
	for reg in regs[1..-1]
		headerFile.puts("\tLYNX_#{reg[:NAME]},")
	end
	
	headerFile.puts("\tLYNX_LAST_FULL=LYNX_#{regs[-1][:NAME]},")
	
	last = nil
	for reg in regs
		for pReg in reg[:PARTIALS]
			headerFile.puts("\tLYNX_#{pReg[:NAME]},")
			last = pReg
		end
	end
	
	headerFile.puts("\tLYNX_LAST_PARTIAL=LYNX_#{last[:NAME]},")
	headerFile.puts("\tLYNX_INVALID,")
	headerFile.puts("\tLYNX_LAST=LYNX_INVALID")
	headerFile.puts("} LynxReg;")
end

def printReg2LynxReg(file, allRegs) 
	file.puts("LynxReg Reg2LynxReg(REG reg) {")
	file.puts("\tswitch(reg) {")
	
	printEnd = false
	for entry in allRegs
		if(entry[:CONDITION][0..4] != "#elif" && printEnd)
			file.puts("#endif") 
			printEnd = false
		end     
		file.puts(entry[:CONDITION]) if entry[:CONDITION] != ""
		printEnd = true if entry[:CONDITION][0..2] == "#if"
		for reg in entry[:REGS]
			file.puts("\t\tcase REG_#{reg}:")
			file.puts("\t\t\treturn LYNX_#{reg};")
		end 
	end
	file.puts("\t\tdefault:")
	file.puts("\t\t\tPIN_MutexLock(&fileWriteLock);")
	file.puts("\t\t\tfprintf(errorFile, \"Unknown Reg: %s\\n\", REG_StringShort(reg));")
	file.puts("\t\t\tPIN_MutexUnlock(&fileWriteLock);")
	file.puts("\t\t\treturn LYNX_INVALID;")
	file.puts("\t}")
	file.puts("}")
end

def genLynxReg2Str(cppFile, headerFile, regs) 
	cppFile.puts("const char *LynxReg2StrArr[] = {")
	
	for reg in regs
		cppFile.puts("\t\"#{reg[:NAME]}\",")
	end
	
	for reg in regs
		for pReg in reg[:PARTIALS]
			cppFile.puts("\t\"#{pReg[:NAME]}\",")
		end
	end
	
	cppFile.puts("\t\"INVALID\"")
	cppFile.puts("};")
	
	headerFile.puts("inline const char *LynxReg2Str(LynxReg lReg);")
	cppFile.puts("inline const char *LynxReg2Str(LynxReg lReg) {")
	cppFile.puts("\treturn LynxReg2StrArr[lReg];")
	cppFile.puts("}")
end

def genLynxRegSize(cppFile, headerFile, regs) 
	cppFile.puts("uint8_t LynxRegSizeArr[] = {")
	
	for reg in regs
		cppFile.puts("\t#{reg[:SIZE]},")
	end
	
	for reg in regs
		for pReg in reg[:PARTIALS]
			cppFile.puts("\t#{pReg[:SIZE]},")
		end
	end
	
	cppFile.puts("\t0")
	cppFile.puts("};")
	
	headerFile.puts("inline uint8_t LynxRegSize(LynxReg lReg);")
	cppFile.puts("inline uint8_t LynxRegSize(LynxReg lReg) {")
	cppFile.puts("\treturn LynxRegSizeArr[lReg];")
	cppFile.puts("}")
end

def genLynxRegOffset(cppFile, headerFile, regs) 
	cppFile.puts("uint8_t LynxRegOffsetArr[] = {")
	
	for reg in regs
		cppFile.puts("\t#{reg[:OFFSET]},")
	end
	
	for reg in regs
		for pReg in reg[:PARTIALS]
			cppFile.puts("\t#{pReg[:OFFSET]},")
		end
	end
	
	cppFile.puts("\t0")
	cppFile.puts("};")
	
	headerFile.puts("inline uint8_t LynxRegOffset(LynxReg lReg);")
	cppFile.puts("inline uint8_t LynxRegOffset(LynxReg lReg) {")
	cppFile.puts("\treturn LynxRegOffsetArr[lReg];")
	cppFile.puts("}")
end

def genLynxReg2FullLynxReg(cppFile, headerFile, regs) 
	cppFile.puts("LynxReg LynxReg2FullLynxRegArr[] = {")
	
	for reg in regs
		cppFile.puts("\tLYNX_#{reg[:NAME]},")
	end
	
	for reg in regs
		for pReg in reg[:PARTIALS]
			cppFile.puts("\tLYNX_#{reg[:NAME]},")
		end
	end
	
	cppFile.puts("\tLYNX_INVALID")
	cppFile.puts("};")
	
	headerFile.puts("inline LynxReg LynxReg2FullLynxReg(LynxReg lReg);")
	cppFile.puts("inline LynxReg LynxReg2FullLynxReg(LynxReg lReg) {")
	cppFile.puts("\treturn LynxReg2FullLynxRegArr[lReg];")
	cppFile.puts("}")
end

if(ARGV.size != 1)
	puts("Usage: gen.rb regsFile")
	exit(1)
end

regFile = File.new(ARGV[0], "r")
regs = []
read(regFile, regs)

headerFile = File.new("LynxReg.h", "w")
cppFile = File.new("LynxReg.c", "w")

headerFile.puts('#ifndef _LYNX_REG_H_')
headerFile.puts('#define _LYNX_REG_H_')
headerFile.puts('#include <stdint.h>')

cppFile.puts('#include "LynxReg.h"')
cppFile.puts()

genEnum(headerFile, regs)
genLynxReg2Str(cppFile, headerFile, regs)
genLynxRegSize(cppFile, headerFile, regs)
genLynxRegOffset(cppFile, headerFile, regs)
genLynxReg2FullLynxReg(cppFile, headerFile, regs)

headerFile.puts("#endif")

puts("There are #{regs.inject(0){|sum,reg| sum+1+reg[:PARTIALS].length()}} registers")
