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

helloworld = """
Main() {
    hello = "Hello World!";
    PRINT(hello);
}
"""

simplebranching = """
Main() {
    aba = "ABA";
    baba = "BABA";
    IF [EQUAL(aba, "ABA")] {
        PRINT(CONCAT("aba equals ", aba));
    } ELIF [EQUAL(baba, "BABA")] {
        PRINT("THIS should't print");
    } ELSE {
        PRINT("Nor this.");
    }
    IF [EQUAL(aba, "BABA")] {
        PRINT("THIS should't print");
    } ELIF [EQUAL(baba, "BABA")] {
        PRINT(CONCAT("baba = BABA: ", EQUAL(baba, "BABA")));
    } ELSE {
        PRINT("Nor this.");
    }
    IF [EQUAL(aba, "BABA")] {
        PRINT("THIS should't print");
    } ELIF [EQUAL(baba, "ABA")] {
        PRINT("Nor this");
    } ELSE {
        PRINT(CONCAT(NOT(EQUAL("abc", "abc")), NOT(EQUAL("def", "efg"))));
    }
}
"""

simpleloops = """
Main() {
    foo = "foo";
    WHILE [NOT(EQUAL(foo, "baz"))] {
        PRINT(foo);
        IF [EQUAL(foo, "foo")] {
            foo = "bar";
        } ELIF [EQUAL(foo, "bar")] {
            foo = "blaz";
        } ELIF [EQUAL(foo, "blaz")] {
            foo = "baz";
        }
    }
    PRINT("done.");
}
"""


simplefunctions = """
FunctionOne(foo, bar) {
    IF [EQUAL(foo, bar)] {
        RETURN "Hello";
    }
    IF [EQUAL(foo, "foo")] {
        RETURN "foo";
    }
    PRINT("Didn't return early!");
}

FunctionTwo(baz) {
    PRINT(FunctionOne(baz, "baz"));
    PRINT(FunctionOne(baz, "bar"));
}

Main() {
    PRINT(FunctionOne("hi", "hi"));
    PRINT(FunctionOne("yo", "yi"));
    PRINT(FunctionOne("foo", "bar"));
    FunctionTwo("bar");
    FunctionTwo("baz");
    FunctionTwo("blowhard");
}

"""

simplevars = """
Foo() {
    PRINT(a);
}

Bar() {
    LOCAL a = "A1 Steak Sauce.";
    IF ["TRUE"] {
        LOCAL a = "Heinz Tomato Ketchup.";
        LOCAL b = "Worchestershire Sauce.";
        PRINT(a);
        PRINT(b);
    }
    PRINT(a);
    PRINT(b);
}

Main() {
    Foo();
    a = "bar";
    Foo();
    a = "barbar";
    Foo();
    LOCAL a = "baz";
    Foo();
    PRINT(a);
    a = "bing";
    Foo();
    Bar();
}
"""

breaks = """

Foo() {
    WHILE ["TRUE"] {
        PRINT("HEYO");
        BREAK;
    }
    PRINT("SUP");
}

Main() {
    WHILE ["TRUE"] {
        PRINT("YO");
        BREAK;
    }
    WHILE ["TRUE"] {
        PRINT("ONE");
        WHILE ["TRUE"] {
            PRINT("TWO");
            Foo();
            BREAK;
            PRINT("HOWDY");
        }
        PRINT("Balderdash");
        BREAK;
    }
    PRINT("SUP");
    RETURN "0";
    PRINT("UNREACHABLE");
}
"""



def main():
    programs = [helloworld, simplebranching, simpleloops, simplefunctions, simplevars, breaks]
    for program in programs:
        if interpreter.InterpretStr(program) == None:
            return


if __name__ == "__main__":
    main()
