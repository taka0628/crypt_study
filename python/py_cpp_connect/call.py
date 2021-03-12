#!/usr/bin/python3

import CModule

str_result = CModule.hello()
print(f"実行結果: {str_result}")

result = CModule.add_cpp(2, 3)
print(f"add_cpp: {result}")