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

import interpreter
import parser
import sys

#TODO read from command line.
def main():
    filenames = ["../poiboi_src/IntMath.poiboi", "../poiboi_src/IntMathTables.poiboi",
                 "../poiboi_src/PoiCore.poiboi", "../poiboi_src/IntMath_test.poiboi"]
    modules = []

    for name in filenames:
        f = file(name, 'r')
        code = ""
        for line in f:
            code += line
        module = parser.ParseCode(code)
        if module == None:
            return None
        modules.append(module)
    print "Compiling."
    interpreter.InterpretModules(modules)


if __name__ == "__main__":
    main()
