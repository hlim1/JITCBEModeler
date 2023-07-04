#!/usr/bin/ruby -w
def read(file, regs)
	regs.push({:CONDITION => "", :REGS => []})
	while(line = file.gets)
		line.strip!
		next if line[0..1] == "/*" || line == ""

		if(line[-1] == ',')
			line[-1] = ""
		end

		if(line[0] == "#")
			if(line == "#endif")
				regs.push({:CONDITION => "", :REGS => []})
			else
				regs.push({:CONDITION => line, :REGS => []})
			end
		else
			regs[-1][:REGS].push(line)
		end
	end
end

def printEnum(file, regs, firstStr, lastStr)
	regs.delete_if{ |entry|
		entry[:REGS].empty?
	}

	first = nil
	
	if(firstStr != "")
		file.puts("\t#{firstStr}=0,")
		file.puts("\tLYNX_#{regs[0][:REGS][0]}=#{firstStr},")
		first = regs[0][:REGS].delete_at(0)
	end

	printEnd = false

	for entry in regs
		if(entry[:CONDITION][0..4] != "#elif" && printEnd)
			file.puts("#endif") 
			printEnd = false
		end
		file.puts(entry[:CONDITION]) if entry[:CONDITION] != ""
		printEnd = true if entry[:CONDITION][0..2] == "#if"
		for reg in entry[:REGS]
			file.puts("\tLYNX_#{reg},")
		end
	end
	
	file.puts("\t#{lastStr}=LYNX_#{regs[-1][:REGS][-1]},")
	
	if(first)
		regs[0][:REGS].insert(0,first)
	end
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

def printLynxReg2StrArr(file, allRegs) 
	file.puts("const char *LynxReg2StrArr[] = {")

	printEnd = false
	for entry in allRegs
		if(entry[:CONDITION][0..4] != "#elif" && printEnd)
			file.puts("#endif")
			printEnd = false
		end
		file.puts(entry[:CONDITION]) if entry[:CONDITION] != ""
		printEnd = true if entry[:CONDITION][0..2] == "#if"
		for reg in entry[:REGS]
			file.puts("\t\"#{reg}\",")
		end
	end
	file.puts("\"INVALID\"")
	file.puts("};")
end

def printLynxReg2RegArr(file, allRegs)
	file.puts("REG LynxReg2RegArr[] = {")

	printEnd = false
	for entry in allRegs
		if(entry[:CONDITION][0..4] != "#elif" && printEnd)
			file.puts("#endif")
			printEnd = false
		end
		file.puts(entry[:CONDITION]) if entry[:CONDITION] != ""
		printEnd = true if entry[:CONDITION][0..2] == "#if"
		for reg in entry[:REGS]
			file.puts("\tREG_#{reg},")
		end
	end
	file.puts("REG_INVALID_")
	file.puts("};")
end

def printLynxRegFcns(cppFile, headerFile)
	headerFile.puts()
	headerFile.puts("LynxReg Reg2LynxReg(REG reg);")
	headerFile.puts("const char *LynxReg2Str(LynxReg lReg);")
	headerFile.puts("REG LynxReg2Reg(LynxReg lReg);")
	headerFile.puts("UINT32 LynxRegSize(LynxReg lReg);")
	headerFile.puts("LynxReg getFullLynxReg(LynxReg r);")
	headerFile.puts()
	cppFile.puts("const char *LynxReg2Str(LynxReg lReg) {")
	cppFile.puts("\treturn LynxReg2StrArr[lReg];")
	cppFile.puts("}")
	cppFile.puts()
	cppFile.puts("REG LynxReg2Reg(LynxReg lReg) {")
	cppFile.puts("\treturn LynxReg2RegArr[lReg];")
	cppFile.puts("}")
	cppFile.puts()
	cppFile.puts("UINT32 LynxRegSize(LynxReg lReg) {")
	cppFile.puts("\treturn REG_Size(LynxReg2RegArr[lReg]);")
	cppFile.puts("}")
	cppFile.puts()
	cppFile.puts("LynxReg getFullLynxReg(LynxReg r) {")
	cppFile.puts("\treturn Reg2LynxReg(REG_FullRegName(LynxReg2Reg(r)));")
	cppFile.puts("}")
	cppFile.puts()
end

if(ARGV.size != 2)
	puts("Usage: gen.rb fullRegsFile partialRegsFile")
	exit(1)
end

fullRegsFile = File.new(ARGV[0], "r")
fullRegs = []
read(fullRegsFile, fullRegs)

partialRegsFile = File.new(ARGV[1], "r")
partialRegs = []

read(partialRegsFile, partialRegs)

headerFile = File.new("LynxReg.h", "w")

headerFile.puts('#ifndef _LYNX_REG_H_')
headerFile.puts('#define _LYNX_REG_H_')
headerFile.puts('#include "pin.H"')
headerFile.puts()
headerFile.puts("typedef enum {")

printEnum(headerFile, fullRegs, "LYNX_FIRST", "LYNX_LAST_FULL")
printEnum(headerFile, partialRegs, "", "LYNX_LAST_PARTIAL")
headerFile.puts("\tLYNX_INVALID,")
headerFile.puts("\tLYNX_LAST=LYNX_INVALID")
headerFile.puts("} LynxReg;\n")

allRegs = fullRegs.dup
allRegs.concat(partialRegs)

headerFile.puts()
headerFile.puts("extern PIN_MUTEX fileWriteLock;")
headerFile.puts("extern FILE *errorFile;")
headerFile.puts('#endif')

cppFile = File.new("LynxReg.cpp", "w")
cppFile.puts('#include "LynxReg.h"')

headerFile.puts()
printLynxReg2StrArr(cppFile, allRegs)
headerFile.puts()
printLynxReg2RegArr(cppFile, allRegs)
printReg2LynxReg(cppFile, allRegs)

cppFile.puts()
printLynxRegFcns(cppFile, headerFile)

cnt = 0
for entry in allRegs
	cnt += entry[:REGS].size
end

puts("There are #{cnt} registers")
