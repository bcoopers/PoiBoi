'''
Copyright 2021 Brian Coopersmith

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

import os

import evaluator
import function
import grammar
import parser
import scanner
import variables

def CompileModules(modules):
    functions_list = function.GetFunctionsListInModules(modules)
    functions_dict = function.GetFunctionsDict(functions_list)
    if functions_dict == None:
        return None
    code = "#define POIBOI_EXECUTABLE_\n"
    code += "#define POIBOI_INCLUDE_ASSERT_\n"
    f = file(os.path.join(os.path.dirname(__file__), '..', 'cc_src', 'poiboi_string.h'))
    for line in f:
        code += line
        f = file(os.path.join(os.path.dirname(__file__), '..', 'cc_src', 'poiboi_string.cc'))
    for line in f:
        code += line
    code += "\n\n\n\n\n"
    for f in functions_list:
        code += f.get_declaration() + ";\n"
    code += "\n\n\n\n\n"

    global_variables = {};
    fn_codes = []
    for f in functions_list:
        val, fn_code = f.compile(global_variables, functions_dict)
        if val == evaluator.evaluationError:
            print "Couldn't compile code: ", fn_code
            return None
        fn_codes.append(fn_code)
    for g in global_variables.keys():
        code += "PBString " + g + "_global_poiboivar;\n"
    code += "\n\n\n\n\n"
    for c in fn_codes:
        code += c
        code += "\n\n\n\n\n"
    if not "Main" in functions_dict.keys():
        code += "int main() { return 0; }"
    else:
        main = functions_dict["Main"]
        if len(main.variables_list) > 1:
            print "Requires either 0 or 1 argument to Main, got " + len(main.variables_list)
            return None
        if len(main.variables_list) == 1:
            # TODO: IO out of main.
            code += "int main() { Main_poiboi_fn(PBString()); return 0; }"
        else:
            code += "int main() { Main_poiboi_fn(); return 0; }"
    return code


def CompileStr(program):
    module = parser.ParseCode(program)
    if module == None:
        return None
    return CompileModules([module])
