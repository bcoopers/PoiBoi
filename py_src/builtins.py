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


evaluationError = -1
asYouWere = 0

class Equal:
    def __init__(self):
        self.name = "EQUAL"
        self.cpp_name = "Builtin_Equal"
        self.num_args = 2
    def evaluate(self, arguments):
        val = "FALSE"
        if arguments[0] == arguments[1]:
            val = "TRUE"
        return val

class Print:
    def __init__(self):
        self.name = "PRINT"
        self.cpp_name = "Builtin_Print"
        self.num_args = 1
    def evaluate(self, arguments):
        #TODO: No newline.
        print arguments[0]
        return arguments[0]

class Concat:
    def __init__(self):
        self.name = "CONCAT"
        self.cpp_name = "Builtin_Concat"
        self.num_args = 2
    def evaluate(self, arguments):
        return arguments[0] + arguments[1]

class Not:
    def __init__(self):
        self.name = "NOT"
        self.cpp_name = "Builtin_Not"
        self.num_args = 1
    def evaluate(self, arguments):
        if arguments[0] == "TRUE":
            return "FALSE"
        return "TRUE"

class And:
    def __init__(self):
        self.name = "AND"
        self.cpp_name = "Builtin_And"
        self.num_args = 2
    def evaluate(self, arguments):
        if arguments[0] == "TRUE" and arguments[1] == "TRUE":
            return "TRUE"
        return "FALSE"

class Or:
    def __init__(self):
        self.name = "OR"
        self.cpp_name = "Builtin_Or"
        self.num_args = 2
    def evaluate(self, arguments):
        if arguments[0] == "TRUE" or arguments[1] == "TRUE":
            return "TRUE"
        return "FALSE"

class Strlen:
    def __init__(self):
        self.name = "STRLEN"
        self.cpp_name = "Builtin_Strlen"
        self.num_args = 1
    def evaluate(self, arguments):
        return str(len(arguments[0]))

class Substring:
    def __init__(self):
        self.name = "SUBSTRING"
        self.cpp_name = "Builtin_Substring"
        self.num_args = 3
    def evaluate(self, arguments):
        try:
            substr_start = int(arguments[1])
        except:
            substr_start = 0
        try:
            substr_end = int(arguments[2])
        except:
            substr_end = len(arguments[0])
        if substr_start < 0:
            substr_start = 0
        if substr_end > len(arguments[0]):
            substr_end = len(arguments[0])
        if substr_start >= substr_end:
            return ""
        return arguments[0][substr_start:substr_end]



# TODO: Should we support find?
def GetAllBuiltins():
    return [Equal(), Print(), Concat(), Not(), And(), Or(), Strlen(),
            Substring()]

def EvaluateBuiltin(name, arguments):
    for builtin in GetAllBuiltins():
        if builtin.name == name:
            if len(arguments) == builtin.num_args:
                return asYouWere, builtin.evaluate(arguments)
            else:
                return evaluationError, "Wrong number of arguments for " + builtin.name + "; expected " + str(builtin.num_args) + ", got " + str(len(arguments))
    else:
        return evaluationError, "No builtin named " + name



def CompileBuiltin(name, arguments):
    for builtin in GetAllBuiltins():
        if builtin.name == name:
            if len(arguments) == builtin.num_args:
                code = builtin.cpp_name + "("
                if len(arguments) > 0:
                    code += arguments[0]
                    if len(arguments) > 1:
                        for arg in arguments[1:]:
                            code += ", " + arg
                code += ")"
                return asYouWere, code
            else:
                return evaluationError, "Wrong number of arguments for " + builtin.name + "; expected " + str(builtin.num_args) + ", got " + str(len(arguments))
    else:
        return evaluationError, "No builtin named " + name
