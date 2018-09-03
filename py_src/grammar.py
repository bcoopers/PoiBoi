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

import scanner

# What represents a valid PoiBoi file. Just a series of function definitions.
class Module:
    def __init__(self):
        self.children = []
        self.content = 'Function Definition, eg: functionDefinition() {}'
    def descendents(self):
        return [
            [scanner.EndOfFile()],
            [FunctionDefinition(), Module()]
        ]

# Function definitions have the name, argument list, and the code which defines
# the function.
class FunctionDefinition:
    def __init__(self):
        self.children = []
        self.content = 'Function Definition, eg: functionDefinition() {}'
    def descendents(self):
        return [
            [scanner.FunctionName(), scanner.OpenFunctionCall(),
             VariablesList(), scanner.CloseFunctionCall(), CodeBlock()]
        ]

# Variable lists are either:
# 1. Empty
# 2. Contain exactly one variable
# 3. Contain multiple variables separated by commas.
class VariablesList:
    def __init__(self):
        self.children = []
        self.content = 'Variables List, eg: a, b, c'
    def descendents(self):
        return [
            [scanner.Variable(), VariablesListExpansion()],
            []
        ]

class VariablesListExpansion:
    def __init__(self):
        self.children = []
        self.content = 'Variables List, eg: a, b, c'
    def descendents(self):
        return [
            [scanner.ArgumentListSeparator(), scanner.Variable(),
             VariablesListExpansion()],
            []
        ]


# A code block is all valid code surrounded by {}.
class CodeBlock:
    def __init__(self):
        self.children = []
        self.content = 'Code Block, eg: {RETURN "foobar";}'
    def descendents(self):
        return [
            [scanner.OpenCodeBlock(), StatementList(), scanner.CloseCodeBlock()]
        ]

# A StatementList is all valid code, or empty.
class StatementList:
    def __init__(self):
        self.children = []
        self.content = 'Valid statement list, eg: foo = "bar"; baz = "bing";'
    def descendents(self):
        return [
            [Statement(), StatementList()],
            []
        ]

# A statement is variable assignment, function call,
# a while loop, an if/else statement, a return statement.
class Statement:
    def __init__(self):
        self.children = []
        self.content = 'Valid statement, eg: foo = "bar";'
    def descendents(self):
        return [
            [VariableAssignment(), scanner.EndStatement()],
            [FunctionCall(), scanner.EndStatement()],
            [scanner.KeywordWhile(), ConditionalEvaluator(), CodeBlock()],
            [scanner.KeywordIf(), ConditionalEvaluator(), CodeBlock(),
             ElseStatement()],
            [scanner.KeywordReturn(), RValue(), scanner.EndStatement()],
            [scanner.KeywordBreak(), scanner.EndStatement()]
        ]

# Either assigns a local or global variable to an RValue.
class VariableAssignment:
    def __init__(self):
        self.children = []
        self.content = 'Variable assignment, eg: foo = "bar";'
    def descendents(self):
        return [
            [scanner.KeywordLocal(), scanner.Variable(), scanner.Assigner(),
             RValue()],
            [scanner.Variable(), scanner.Assigner(), RValue()]
        ]

# Call a function or builtin with arguments surrounded by ().
class FunctionCall:
    def __init__(self):
        self.children = []
        self.content = 'Function or builtin call, eg: PRINT("foobar")'
    def descendents(self):
        return [
            [scanner.FunctionName(), scanner.OpenFunctionCall(),
             RValueList(), scanner.CloseFunctionCall()],
            [scanner.Builtin(), scanner.OpenFunctionCall(),
             RValueList(), scanner.CloseFunctionCall()],
        ]

# [RValue]
class ConditionalEvaluator:
    def __init__(self):
        self.children = []
        self.content = 'Conditional evaluator, eg: ["TRUE"]'
    def descendents(self):
        return [
            [scanner.OpenControlBlock(), RValue(), scanner.CloseControlBlock()]
        ]

# Can be empty, or an ELSE{}, or an ELSE IF [] {}
class ElseStatement:
    def __init__(self):
        self.children = []
        self.content = 'Else statement, eg: ELIF ["TRUE"] {}'
    def descendents(self):
        return [
            [scanner.KeywordElse(), CodeBlock()],
            [scanner.KeywordElif(), ConditionalEvaluator(), CodeBlock(),
             ElseStatement()],
            []
        ]

# An rvalue is something which returns a string. So a string is fine, or
# a variable, or a function call.
class RValue:
    def __init__(self):
        self.children = []
        self.content = 'An RValue- either a string, a variable, or a function call. eg: "TRUE"'
    def descendents(self):
        return [
            [scanner.String()],
            [scanner.Variable()],
            [FunctionCall()]
        ]


# An RValue list is either empty, contains one RValue, or multiple RValues
# separated by ,
class RValueList:
    def __init__(self):
        self.children = []
        self.content = 'A list of RValue- strings, variables, and function calls. eg: "TRUE", EQUAL("1", "2"), foo("BAR")'
    def descendents(self):
        return [
            [RValue(), RValueListExpansion()],
            []
        ]

class RValueListExpansion:
    def __init__(self):
        self.children = []
        self.content = 'A list of RValue- strings, variables, and function calls. eg: "TRUE", EQUAL("1", "2"), foo("BAR")'
    def descendents(self):
        return [
            [scanner.ArgumentListSeparator(), RValue(), RValueListExpansion()],
            []
        ]
