# IDAExportFunctions
 
IDA C++ plugin that can export virtual functions and non virtual functions grouped by classes to json.

JSON Structure
{
	classes:
	[{
		className: "",
		inheritedClasses: [],
		virtualFunctionTables:
		[
			{
				forClass: "",
				virtualFunctions:
				[
					prototype: "",
					functionName: "",
					address: "",
					callingConvention: "",
					returnType: "",
					parameters:
					[
						{
							type: "",
							name: ""
						}
					],
					virtualTableIndex: 0
				]
			}
		],
		nonVirtualfunctions:
		[
			{
				prototype: "",
				functionName: "",
				address: "",
				callingConvention: "",
				returnType: "",
				parameters:
				[
					{
						type: "",
						name: ""
					}
				]
			}
		]
	}]
}
