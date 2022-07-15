

class Assembler(val code: String){

	private var line: Int = 0
	private var char: Int = 0

	private fun scanLabel(){


	}

	private fun assembleLine(code: String){

		this.char = 0
	}

	@OptIn(kotlin.ExperimentalUnsignedTypes::class)
	public fun assemble(): UByteArray {

		this.line = 0
	
		for(line in code.split('\n')){
			assembleLine(line)
			this.line++
		}
		
		return ubyteArrayOf(0x00u)
	}
}



class CommandLineArguments(val rawArgs: Array<String>,
	val flagNames: MutableSet<String> = mutableSetOf<String>(),
	val valueArgNames: MutableSet<String> = mutableSetOf<String>()
){
	var positionals = mutableListOf<String>()
	var flags = mutableListOf<String>()
	var valueArgs = mutableMapOf<String, String>()

	init {
		var pendingValueArg: String? = null

		for(arg in rawArgs){
			if(pendingValueArg != null){
				this.valueArgs[pendingValueArg] = arg
			} else when(arg){
				in flagNames -> flags.add(arg)
				in valueArgNames -> pendingValueArg = arg
				else -> positionals.add(arg)
			}
		}
	}
}

fun main(args: Array<String>){

	val cla = CommandLineArguments(
		rawArgs = args,
		flagNames = mutableSetOf<String>(),
		valueArgNames = mutableSetOf<String>("-o")
	)


}