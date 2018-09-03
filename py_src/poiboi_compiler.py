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

import sys

import compiler
import parser

def main():
    if len(sys.argv) < 3:
        print "The last argument is the output file. The preceeding arguments are input PoiBoi source code."
        return 0
    filenames = sys.argv[1:-1]
    outfile = sys.argv[-1]
    if outfile.endswith(".poiboi"):
        print "The last argument should be an output C++ file, not an input PoiBoi file."
        return 0
    modules = []

    for name in filenames:
        print "Lexing and parsing", name
        f = file(name, 'r')
        code = ""
        for line in f:
            code += line
        module = parser.ParseCode(code)
        if module == None:
            return None
        modules.append(module)
    print "Compiling program"
    code = compiler.CompileModules(modules)
    if code != None:
        out = file(outfile, 'w')
        out.write(code)
        out.close()


if __name__ == "__main__":
    main()
