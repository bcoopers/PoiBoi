'''
Copyright 2018 Brian Coopersmith

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    https://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
'''

import evaluator
import function
import grammar
import parser
import scanner
import variables

def InterpretModules(modules, commandinput = ""):
    functions_list = function.GetFunctionsListInModules(modules)
    functions_dict = function.GetFunctionsDict(functions_list)
    if functions_dict == None:
        return None
    if not "Main" in functions_dict.keys():
        return ""
    main = functions_dict["Main"]
    init_local_variables_dict = {}
    if len(main.variables_list) > 1:
        print "Requires either 0 or 1 argument to Main, got " + len(main.variables_list)
        return None
    call_values = []
    if len(main.variables_list) == 1:
        call_values = [commandinput]
    print "Calling main."
    code, payload = main.call(
        commandinput, {}, functions_dict)
    if code == evaluator.evaluationError:
        print "Couldn't interpret code: ", payload
        return None
    return payload


def InterpretStr(program, commandinput = ""):
    module = parser.ParseCode(program)
    if module == None:
        return None
    return InterpretModules([module])

if __name__ == "__main__":
    main()
