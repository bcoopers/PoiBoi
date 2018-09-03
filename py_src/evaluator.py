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

import builtins
import grammar
import scanner
import variables

evaluationError = builtins.evaluationError
asYouWere = builtins.asYouWere
returnFromFunction = 1
breakFromLoop = 2

def getRValuesFromExpansion(rvalue_list_expansion):
    rvalues = []
    while rvalue_list_expansion != None:
        children = rvalue_list_expansion.children
        if len(children) == 0:
            return rvalues
        assert isinstance(children[1], grammar.RValue)
        rvalues.append(RValueEvaluator(children[1]))
        assert isinstance(children[2], grammar.RValueListExpansion)
        rvalue_list_expansion = children[2]

def getRValuesFromList(rvalue_list):
    assert isinstance(rvalue_list, grammar.RValueList)
    children = rvalue_list.children
    if len(children) == 0:
        return []
    assert isinstance(children[0], grammar.RValue)
    assert isinstance(children[1], grammar.RValueListExpansion)
    return [RValueEvaluator(children[0])] + getRValuesFromExpansion(children[1])

class FunctionCaller:
    def __init__(self, function_call):
        assert isinstance(function_call, grammar.FunctionCall)
        children = function_call.children
        self.function_name = children[0].content
        self.is_builtin = isinstance(children[0], scanner.Builtin)
        self.rvalue_evaluators = getRValuesFromList(children[2])

    def evaluate(self, local_variables, global_variables, functions_dict,
                 is_in_loop = False):
        call_values = []
        for evaluator in self.rvalue_evaluators:
            rvalue_result, payload = evaluator.evaluate(
                local_variables, global_variables, functions_dict)
            if rvalue_result == evaluationError:
                return evaluationError, payload
            call_values.append(payload)
        if self.is_builtin:
            return builtins.EvaluateBuiltin(self.function_name, call_values)
        elif self.function_name in functions_dict.keys():
            function = functions_dict[self.function_name]
            result, payload = function.call(call_values, global_variables,
                                            functions_dict)
            if result == evaluationError:
                return evaluationError, payload
            return asYouWere, payload
        return evaluationError, "Function " + self.function_name + " not defined."
    def compile(self, local_variables, global_variables, functions_dict,
                is_in_loop = False):
        call_values = []
        for evaluator in self.rvalue_evaluators:
            rvalue_result, rvalue_code = evaluator.compile(
                local_variables, global_variables, functions_dict)
            if rvalue_result == evaluationError:
                return evaluationError, rvalue_code
            call_values.append(rvalue_code)
        if self.is_builtin:
            return builtins.CompileBuiltin(self.function_name, call_values)
        elif self.function_name in functions_dict.keys():
            function = functions_dict[self.function_name]
            if len(call_values) != len(function.variables_list):
                return evaluationError, "Calling function " + self.function_name + " with " + str(len(call_values)) + " args, expected " + str(len(function.variables_list))

            code = self.function_name + "_poiboi_fn" + "("
            if len(call_values) > 0:
                code += call_values[0]
                if len(call_values) > 1:
                    for arg in call_values[1:]:
                        code += ", " + arg
            code += ")"
            return asYouWere, code
        return evaluationError, "Function " + self.function_name + " not defined."

class RValueEvaluator:
    def __init__(self, rvalue):
        assert isinstance(rvalue, grammar.RValue)
        children = rvalue.children
        assert len(children) == 1
        self.string_content = None
        self.variable_name = None
        self.function_caller = None
        if isinstance(children[0], scanner.String):
            self.string_content = children[0].content
        elif isinstance(children[0], scanner.Variable):
            self.variable_name = children[0].content
        else:
            assert isinstance(children[0], grammar.FunctionCall)
            self.function_caller = FunctionCaller(children[0])
    def evaluate(self, local_variables, global_variables, functions_dict,
                       is_in_loop = False):
        if self.string_content != None:
            return asYouWere, eval(self.string_content)
        elif self.variable_name != None:
            local_var_value = local_variables.get_value(self.variable_name)
            if local_var_value != None:
                return asYouWere, local_var_value
            elif self.variable_name in global_variables.keys():
                return asYouWere, global_variables[self.variable_name]
            else:
                return asYouWere, ""
        else:
            assert self.function_caller != None
            return self.function_caller.evaluate(
                local_variables, global_variables, functions_dict)
    def compile(self, local_variables, global_variables, functions_dict,
                is_in_loop = False):
        if self.string_content != None:
            return asYouWere, "PBString::NewStaticString(" + self.string_content + ")"
        elif self.variable_name != None:
            local_var_value = local_variables.get_value(self.variable_name)
            if local_var_value != None:
                return asYouWere, self.variable_name + "_local_poiboivar"
            elif self.variable_name in global_variables.keys():
                return asYouWere, self.variable_name + "_global_poiboivar"
            else:
                return asYouWere, "PBString()"
        else:
            assert self.function_caller != None
            return self.function_caller.compile(
                local_variables, global_variables, functions_dict)


class VariableAssigner:
    def __init__(self, variable_assignment):
        self.is_local = False
        children = variable_assignment.children
        if len(children) == 4:
            assert isinstance(children[0], scanner.KeywordLocal)
            self.is_local = True
            children = children[1:]
        assert len(children) == 3
        self.variable_name = children[0].content
        self.rvalue_evaluator = RValueEvaluator(children[2])
    def evaluate(self, local_variables, global_variables, functions_dict,
                 is_in_loop = False):
        is_local = self.is_local or local_variables.get_value(self.variable_name) != None
        rvalue_result, payload = self.rvalue_evaluator.evaluate(
            local_variables, global_variables, functions_dict)
        if rvalue_result == evaluationError:
            return evaluationError, payload
        if is_local:
            local_variables.set_value(self.variable_name, payload)
        else:
            global_variables[self.variable_name] = payload
        return asYouWere, ""
    def compile(self, local_variables, global_variables, functions_dict,
                is_in_loop):
        is_local = self.is_local or local_variables.get_value(self.variable_name) != None
        new_local = is_local and local_variables.get_value(self.variable_name) == None
        result, rvaluecode = self.rvalue_evaluator.compile(
            local_variables, global_variables, functions_dict)
        if result == evaluationError:
            return result, rvaluecode
        if new_local:
            local_variables.set_value(self.variable_name, "1")
            return asYouWere, "PBString " + self.variable_name + "_local_poiboivar = " + rvaluecode + ";"
        elif is_local:
            return asYouWere, self.variable_name + "_local_poiboivar = " + rvaluecode + ";"
        else:
            global_variables[self.variable_name] = "bleh"
            return asYouWere, self.variable_name + "_global_poiboivar = " + rvaluecode + ";"

def IsTrue(variable):
    return variable == "TRUE"

class IfEvaluator:
    def __init__(self, conditional_evaluator, code_block, else_statement):
        self.conditional_rvalue_evaluator = (
            RValueEvaluator(conditional_evaluator.children[1]))
        self.if_evaluator = StatementsEvaluator(code_block.children[1])
        self.else_evaluator = None
        else_children = else_statement.children
        if len(else_children) > 0:
            if isinstance(else_children[0], scanner.KeywordElse):
                self.else_evaluator = StatementsEvaluator(
                    else_children[1].children[1])
            else:
                assert isinstance(else_children[0], scanner.KeywordElif)
                self.else_evaluator = IfEvaluator(
                    else_children[1], else_children[2], else_children[3])
    def evaluate(self, local_variables, global_variables, functions_dict,
                 is_in_loop):
        conditional_result, payload = self.conditional_rvalue_evaluator.evaluate(
            local_variables, global_variables, functions_dict)
        if conditional_result == evaluationError:
            return conditional_result, payload
        evaluator = self.else_evaluator
        if IsTrue(payload):
            evaluator = self.if_evaluator
        if evaluator != None:
            new_locals = variables.LocalVariables(parent=local_variables)
            result, payload = evaluator.evaluate(
                new_locals, global_variables, functions_dict, is_in_loop)
            if result in [evaluationError, returnFromFunction, breakFromLoop]:
                return result, payload
        return asYouWere, ""
    def compile(self, local_variables, global_variables, functions_dict,
                is_in_loop):
        result, conditional = self.conditional_rvalue_evaluator.compile(
            local_variables, global_variables, functions_dict)
        if result == evaluationError:
            return result, conditional
        new_locals = variables.LocalVariables(parent=local_variables)
        result, in_if = self.if_evaluator.compile(
            new_locals, global_variables, functions_dict, is_in_loop)
        if result == evaluationError:
            return result, in_if
        in_else = None
        if self.else_evaluator != None:
            new_locals = variables.LocalVariables(parent=local_variables)
            result, in_else = self.else_evaluator.compile(
                new_locals, global_variables, functions_dict, is_in_loop)
            if result == evaluationError:
                return result, in_else
        code = "if (" + conditional + ") {\n"
        code += in_if
        code += '}'
        if in_else == None:
            code += '\n'
        else:
            code += ' else {\n' + in_else + '}\n'
        return asYouWere, code

class WhileEvaluator:
    def __init__(self, conditional_evaluator, code_block):
        self.conditional_rvalue_evaluator = (
            RValueEvaluator(conditional_evaluator.children[1]))
        self.statements_evaluator = StatementsEvaluator(code_block.children[1])
    def evaluate(self, local_variables, global_variables, functions_dict,
                 is_in_loop = True):
        while True:
            conditional_result, payload = self.conditional_rvalue_evaluator.evaluate(
                local_variables, global_variables, functions_dict)
            if conditional_result == evaluationError:
                return conditional_result, payload
            elif not IsTrue(payload):
                return asYouWere, ""
            new_locals = variables.LocalVariables(parent=local_variables)
            result, payload = self.statements_evaluator.evaluate(
                new_locals, global_variables, functions_dict, True)
            if result in [evaluationError, returnFromFunction]:
                return result, payload
            if result == breakFromLoop:
                return asYouWere, payload
    def compile(self, local_variables, global_variables, functions_dict,
                is_in_loop):
        result, conditional = self.conditional_rvalue_evaluator.compile(
            local_variables, global_variables, functions_dict)
        if result == evaluationError:
            return result, conditional
        new_locals = variables.LocalVariables(parent=local_variables)
        result, in_loop = self.statements_evaluator.compile(
            new_locals, global_variables, functions_dict, True)
        if result == evaluationError:
            return result, in_loop
        return asYouWere, "while (" + conditional + ") {\n" + in_loop + "}\n"

class BreakEvaluator:
    def evaluate(self, local_variables, global_variables, functions_dict,
                 is_in_loop):
        if is_in_loop:
            return breakFromLoop, ""
        return evaluationError, "Cannot have BREAK statement outside loop."
    def compile(self, local_variables, global_variables, functions_dict,
                is_in_loop):
        if is_in_loop:
            return asYouWere, "break;\n"
        else:
            return evaluationError, "Cannot have BREAK statement outside loop."

class ReturnEvaluator:
    def __init__(self, rvalue):
        self.rvalue_evaluator = RValueEvaluator(rvalue)
    def evaluate(self, local_variables, global_variables, functions_dict,
                 is_in_loop = False):
        rvalue_result, payload = self.rvalue_evaluator.evaluate(
            local_variables, global_variables, functions_dict)
        if rvalue_result == evaluationError:
            return evaluationError, payload
        return returnFromFunction, payload
    def compile(self, local_variables, global_variables, functions_dict,
                is_in_loop):
        result, payload = self.rvalue_evaluator.compile(
            local_variables, global_variables, functions_dict)
        if payload == evaluationError:
            return evaluationError, payload
        return asYouWere, "return " + payload + ";\n"

class StatementsEvaluator:
    def __init__(self, statement_list):
        assert isinstance(statement_list, grammar.StatementList)
        self.statements = []
        children = statement_list.children
        while len(children) > 0:
            statement = children[0]
            if isinstance(statement.children[0], grammar.VariableAssignment):
                self.statements.append(VariableAssigner(statement.children[0]))
            elif isinstance(statement.children[0], grammar.FunctionCall):
                self.statements.append(FunctionCaller(statement.children[0]))
            elif isinstance(statement.children[0], scanner.KeywordWhile):
                self.statements.append(WhileEvaluator(
                    statement.children[1], statement.children[2]))
            elif isinstance(statement.children[0], scanner.KeywordIf):
                self.statements.append(IfEvaluator(
                    statement.children[1], statement.children[2],
                    statement.children[3]))
            elif isinstance(statement.children[0], scanner.KeywordBreak):
                self.statements.append(BreakEvaluator())
            else:
                assert isinstance(statement.children[0], scanner.KeywordReturn)
                self.statements.append(ReturnEvaluator(statement.children[1]))
            children = children[1].children

    def evaluate(self, local_variables, global_variables, functions_dict,
                 is_in_loop):
        for statement in self.statements:
            result, payload = statement.evaluate(
                local_variables, global_variables, functions_dict, is_in_loop)
            if result in [evaluationError, returnFromFunction, breakFromLoop]:
                return result, payload
        return asYouWere, ''
    def compile(self, local_variables, global_variables, functions_dict,
                is_in_loop):
        code = ""
        for statement in self.statements:
            result, statement_code = statement.compile(
                local_variables, global_variables, functions_dict, is_in_loop)
            if result == evaluationError:
                return result, statement_code
            code += statement_code
            if isinstance(statement, FunctionCaller):
                code += ";\n"
        return asYouWere, code
