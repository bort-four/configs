#!/usr/bin/python

import sys


if len(sys.argv) < 2:
    print('class name is not specified')
    exit

className = sys.argv[1]
headerMacro = className.upper() + '_H'

headerBegin = """// {0}.h

#ifndef {1}
#define {1}

#include <memory>
""".format(className, headerMacro)

headerEnd = '#endif  // {0}\n'.format(headerMacro)

nameSpaceOpen = ''
nameSpaceClose = ''

if len(sys.argv) >= 3:
	nameSpaceOpen = 'namespace {0}\n{{\n'.format(sys.argv[2])
	nameSpaceClose = '}}  // namespace {0}\n'.format(sys.argv[2])

classDefenition = """
class {0};
using {0}Pointer = std::shared_ptr<{0}>;


class {0}
{{
public:
    {0}();
    {0}({0}&& other);
    virtual {0}& operator=({0}&& other);
    virtual ~{0}();

private:
    struct Impl;
    std::unique_ptr<Impl> _pimpl;
}};
""".format(className)


sourceBegin = """// {0}.cpp

#include "{0}.h"
""".format(className)

classImpl = """
struct {0}::Impl
{{
    Impl()
    {{
    }}
}};



{0}::{0}()
    : _pimpl(new Impl())
{{
	// ...
}}

{0}::{0}({0}&& /*other*/) = default;
{0}& {0}::operator=({0}&& /*other*/) = default;
{0}::~{0}() = default;
""".format(className)


header = (headerBegin + '\n\n'
		+ nameSpaceOpen + '\n'
		+ classDefenition + '\n\n'
		+ nameSpaceClose + '\n'
		+ headerEnd);

source = (sourceBegin + '\n\n'
		+ nameSpaceOpen + '\n'
		+ classImpl + '\n\n'
		+ nameSpaceClose);

#~ print('*****************************')
#~ print(header)
#~ print('*****************************')
#~ print(source)
#~ print('*****************************')

headerFile = open('{0}.h'.format(className), 'w')
headerFile.write(header)
headerFile.close()

sourceFile = open('{0}.cpp'.format(className), 'w')
sourceFile.write(source)
sourceFile.close()
