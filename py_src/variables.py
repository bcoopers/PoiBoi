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

class LocalVariables:
    def __init__(self, parent = None, initial_vars = {}):
        self.vars_dict = initial_vars.copy()
        self.parent = parent
    def get_value(self, varname):
        if varname in self.vars_dict:
            return self.vars_dict[varname]
        if self.parent != None:
            return self.parent.get_value(varname)
        return None
    def set_value(self, varname, value):
        if varname in self.vars_dict.keys():
            if self.parent != None and self.parent.get_value(varname) != None:
                print varname
                print self.vars_dict
                print self.parent.vars_dict
                assert self.parent == None or self.parent.get_value(varname) == None
            self.vars_dict[varname] = value
        elif self.parent != None and self.parent.get_value(varname) != None:
            self.parent.set_value(varname, value)
        else:
            self.vars_dict[varname] = value
