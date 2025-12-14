#ifndef LIA_CONSOLE_H
#define LIA_CONSOLE_H

#include "lia_settings.h"   // 提供 huart2
#include <stdarg.h>
#include <stdint.h>
#include <stdbool.h>

/* ====== 简易 console 接口（阻塞式） ====== */

/* 直接发送字符串（不自动附加换行） */
void LiaConsole_Print(const char* s);

/* printf 风格（不自动附加换行） */
void LiaConsole_Printf(const char* fmt,...);

/* 读一行（阻塞）。返回有效长度（不含 '\0'），忽略空行。
 * 行结束符：'\r' 或 '\n'（兼容 CR/LF）。
 * 支持退格（Backspace 0x08 / DEL 0x7F）与回显。 */
int LiaConsole_ReadLine(char* buf, uint32_t maxlen);

/* 打印提示，读取 float（阻塞），返回 0 成功，非 0 失败 */
int LiaConsole_ReadFloat(const char* prompt, float* out_val);

/* 可选：打开/关闭回显（默认开启） */
void LiaConsole_SetEcho(bool enable);

/* ====== 兼容映射（保留旧 API 名称） ====== */
#define Serial_Print      LiaConsole_Print
#define Serial_Printf     LiaConsole_Printf
#define Serial_ReadLine   LiaConsole_ReadLine
#define Serial_ReadFloat  LiaConsole_ReadFloat

#endif /* LIA_CONSOLE_H */
