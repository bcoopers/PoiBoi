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

import grammar
import scanner
import evaluator
import variables

def getVariablesFromExpansion(variables_list_expansion):
    variables = []
    while variables_list_expansion != None:
        children = variables_list_expansion.children
        if len(children) == 0:
            return variables
        assert isinstance(children[1], scanner.Variable)
        variables.append(children[1].content)
        assert isinstance(children[2], grammar.VariablesListExpansion)
        variables_list_expansion = children[2]

def getVariablesFromVariablesList(variables_list):
    children = variables_list.children
    if len(children) == 0:
        return []
    assert isinstance(children[0], scanner.Variable)
    assert isinstance(children[1], grammar.VariablesListExpansion)
    return [children[0].content] + getVariablesFromExpansion(children[1])

class Function:
    def __init__(self, function_definition):
        children = function_definition.children
        assert isinstance(children[0], scanner.FunctionName)
        self.name = children[0].content
        assert isinstance(children[2], grammar.VariablesList)
        self.variables_list = getVariablesFromVariablesList(children[2])
        self.statements = evaluator.StatementsEvaluator(
            children[4].children[1])
    def call(self, call_values, global_variables, functions_dict):
        if len(call_values) != len(self.variables_list):
            return evaluator.evaluationError, "Can't call function " + self.name + "; expected " + str(len(self.variables_list)) + " arguments, got " + str(len(call_values))
        initial_local_vars_dict = {}
        for i in range(len(call_values)):
            initial_local_vars_dict[self.variables_list[i]] = call_values[i]
        local_variables = variables.LocalVariables(initial_vars=initial_local_vars_dict)
        return self.statements.evaluate(local_variables, global_variables,
                                        functions_dict, False)
    def get_declaration(self):
        code = "PBString " + self.name + "_poiboi_fn("
        if len(self.variables_list) > 0:
            code += "PBString " + self.variables_list[0] + "_local_poiboivar"
            if len(self.variables_list) > 1:
                for arg in self.variables_list[1:]:
                    code += ", PBString " + arg + "_local_poiboivar"
        code += ")"
        return code
    def compile(self, global_variables, functions_dict):
        initial_local_vars_dict = {}
        for i in range(len(self.variables_list)):
            initial_local_vars_dict[self.variables_list[i]] = "bleh"
        local_variables = variables.LocalVariables(initial_vars=initial_local_vars_dict)
        code = self.get_declaration() +  "{\n"
        val, innercode = self.statements.compile(local_variables, global_variables,
                                                 functions_dict, False)
        if val == evaluator.evaluationError:
            return val, innercode
        code += innercode + "\nreturn PBString();\n}\n"
        return val, code



def getFunctionsListInModule(module):
    functions = []
    while module != None:
        for child in module.children:
            if isinstance(child, grammar.FunctionDefinition):
                functions.append(Function(child))
            elif isinstance(child, grammar.Module):
                module = child
            elif isinstance(child, scanner.EndOfFile):
                module = None
    return functions

def GetFunctionsListInModules(modules):
    functions = []
    for module in modules:
        functions += getFunctionsListInModule(module)
    return functions

def GetFunctionsDict(functions_list):
    functions_dict = {}
    for function in functions_list:
        if functions_dict.has_key(function.name):
            print "Error: Multiple definitions of function:", function.name
            return None
        functions_dict[function.name] = function
    return functions_dict
