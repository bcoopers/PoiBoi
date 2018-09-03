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

import copy
import re

# Encloses a block of code following a looping control block, or a function
# declaration.
class OpenCodeBlock:
    def __init__(self, content = '{', linenum = -1):
        self.content = content
        self.line_num = linenum
        assert self.search(content) == len(content)
    def search(self, code):
        return 1 if code[0] == '{' else -1

class CloseCodeBlock:
    def __init__(self, content = '}', linenum = -1):
        self.content = content
        self.line_num = linenum
        assert self.search(content) == len(content)
    def search(self, code):
        return 1 if code[0] == '}' else -1

# Ends each "Line of Code".
class EndStatement:
    def __init__(self, content = ';', linenum = -1):
        self.content = content
        self.line_num = linenum
        assert self.search(content) == len(content)
    def search(self, code):
        return 1 if code[0] == ';' else -1

# Surrounds arguments list, both when delaring and calling a function.
class OpenFunctionCall:
    def __init__(self, content = '(', linenum = -1):
        self.content = content
        self.line_num = linenum
        assert self.search(content) == len(content)
    def search(self, code):
        return 1 if code[0] == '(' else -1

class CloseFunctionCall:
    def __init__(self, content = ')', linenum = -1):
        self.content = content
        self.line_num = linenum
        assert self.search(content) == len(content)
    def search(self, code):
        return 1 if code[0] == ')' else -1

# Separates arguments when defining and calling a function.
class ArgumentListSeparator:
    def __init__(self, content = ',', linenum = -1):
        self.content = content
        self.line_num = linenum
        assert self.search(content) == len(content)
    def search(self, code):
        return 1 if code[0] == ',' else -1

# Surrounds control blocks of if statements and while loops.
class OpenControlBlock:
    def __init__(self, content = '[', linenum = -1):
        self.content = content
        self.line_num = linenum
        assert self.search(content) == len(content)
    def search(self, code):
        return 1 if code[0] == '[' else -1

class CloseControlBlock:
    def __init__(self, content = ']', linenum = -1):
        self.content = content
        self.line_num = linenum
        assert self.search(content) == len(content)
    def search(self, code):
        return 1 if code[0] == ']' else -1

# Assigns a variable to a string.
class Assigner:
    def __init__(self, content = '=', linenum = -1):
        self.content = content
        self.line_num = linenum
        assert self.search(content) == len(content)
    def search(self, code):
        return 1 if code[0] == '=' else -1

# Various control flow keywords. Looks like a builtin but is special for
# parsing.
class KeywordLocal:
    def __init__(self, content = 'LOCAL', linenum = -1):
        self.content = content
        self.line_num = linenum
        assert self.search(content) == len(content)
    def search(self, code):
        return 5 if code.startswith('LOCAL') else -1

class KeywordWhile:
    def __init__(self, content = 'WHILE', linenum = -1):
        self.content = content
        self.line_num = linenum
        assert self.search(content) == len(content)
    def search(self, code):
        return 5 if code.startswith('WHILE') else -1

class KeywordIf:
    def __init__(self, content = 'IF', linenum = -1):
        self.content = content
        self.line_num = linenum
        assert self.search(content) == len(content)
    def search(self, code):
        return 2 if code.startswith('IF') else -1

class KeywordElse:
    def __init__(self, content = 'ELSE', linenum = -1):
        self.content = content
        self.line_num = linenum
        assert self.search(content) == len(content)
    def search(self, code):
        return 4 if code.startswith('ELSE') else -1

class KeywordElif:
    def __init__(self, content = 'ELIF', linenum = -1):
        self.content = content
        self.line_num = linenum
        assert self.search(content) == len(content)
    def search(self, code):
        return 4 if code.startswith('ELIF') else -1

class KeywordReturn:
    def __init__(self, content = 'RETURN', linenum = -1):
        self.content = content
        self.line_num = linenum
        assert self.search(content) == len(content)
    def search(self, code):
        return 6 if code.startswith('RETURN') else -1

class KeywordBreak:
    def __init__(self, content = 'BREAK', linenum = -1):
        self.content = content
        self.line_num = linenum
        assert self.search(content) == len(content)
    def search(self, code):
        return 5 if code.startswith('BREAK') else -1

# Strings are one or more sets of characters enclosed in "quotes".
class String:
    def __init__(self, content = '"ex"', linenum = -1):
        self.content = content
        self.line_num = linenum
        assert self.search(content) == len(content)
    def search(self, code):
        # TODO: Support even number of \ before "
        match = re.search('(^(\\"(.*?)(?<!\\\\)\\")+)', code)
        if match != None:
            return len(match.group(0))
        return -1

#Variables are likeself.
class Variable:
    def __init__(self, content = 'ex', linenum = -1):
        self.content = content
        self.line_num = linenum
        assert self.search(content) == len(content)
    def search(self, code):
        match = re.search('(^[a-z][a-zA-Z]*)', code)
        if match != None:
            return len(match.group(0))
        return -1

# Builtins are LIKEself.
class Builtin:
    def __init__(self, content = 'EX', linenum = -1):
        self.content = content
        self.line_num = linenum
        assert self.search(content) == len(content)
    def search(self, code):
        match = re.search('(^[A-Z][A-Z]+)', code)
        if match != None:
            return len(match.group(0))
        return -1

# Function names are LikeThis.
class FunctionName:
    def __init__(self, content = 'Ex', linenum = -1):
        self.content = content
        self.line_num = linenum
        assert self.search(content) == len(content)
    def search(self, code):
        match = re.search('(^[A-Z][a-zA-Z]*[a-z]+[a-zA-Z]*)', code)
        if match != None:
            return len(match.group(0))
        return -1

# Comments start with a # and end with a #.
class Comment:
    def __init__(self, content = '##', linenum = -1):
        self.content = content
        self.line_num = linenum
        assert self.search(content) == len(content)
    def search(self, code):
        match = re.search('(^#.*#)', code)
        if match != None:
            return len(match.group(0))
        return -1

class Whitespace:
    def __init__(self, content = ' ', linenum = -1):
        self.content = content
        self.line_num = linenum
        assert self.search(content) == len(content)
    def search(self, code):
        match = re.search('\\s*', code)
        if match != None:
            return len(match.group(0))
        return -1

class EndOfFile:
    def __init__(self, content = 'EOF', linenum = -1):
        self.content = content
        self.line_num = linenum
    # Only token with no search.

def allTokenTypes():
    return [
        OpenCodeBlock(), CloseCodeBlock(), EndStatement(),
        OpenFunctionCall(), CloseFunctionCall(),
        ArgumentListSeparator(), OpenControlBlock(),
        CloseControlBlock(), Assigner(), KeywordLocal(),
        KeywordWhile(), KeywordIf(), KeywordElse(), KeywordElif(),
        KeywordReturn(), KeywordBreak(), String(), Variable(), Builtin(),
        FunctionName(), Comment(), Whitespace(), EndOfFile()
    ]

def IsToken(item):
    for element in allTokenTypes():
        if element.__class__ == item.__class__:
            return True
    return False

def Scan(code):
    original_code = code
    tokens = []
    linenum = 1
    all_token_types = allTokenTypes()
    while len(code) > 0:
        for element in all_token_types:
            if isinstance(element, EndOfFile):
                continue
            match_length = element.search(code)
            if match_length > 0:
                match_str = code[0:match_length]
                if not isinstance(element, Whitespace) and not isinstance(element, Comment):
                    tokens.append(element.__class__(match_str, linenum))
                code = code[match_length:]
                linenum += match_str.count('\n')
                break
        else:
            print 'Couldn\'t scan code: line ', str(linenum)
            print original_code.split('\n')[linenum - 1]
            return None
    tokens.append(EndOfFile())
    return tokens
