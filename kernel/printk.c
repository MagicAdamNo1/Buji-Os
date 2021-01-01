#include <stdarg.h>
#include "printk.h"
#include "lib.h"
#include "linkage.h"

int color_printk(unsigned int FRcolor,unsigned int BKcolor,const char* fmt,...) {
    int i = 0;
    int count = 0;
    int line = 0;
    va_list args;
    va_start(args,fmt);
    i = vsprintf(buf,fmt,args);
    va_end(args);

    for (count = 0;count < i||line;count++) {
        if (line > 0) {
            count--;
            goto    Label_tab;
        }
        if ((unsigned char)*(buf + count) == '\n'){//如果发现某个待显示字符是\n转义符
            Pos.YPosition++;    // 则将光标行数加1,
            Pos.XPosition = 0;  // 列数设置为0 
        }else if ((unsigned char)*(buf+count) == '\b') {//如果确定待显示字符是\b转义符，
            Pos.XPosition--;                            //那么调整列位置
            if (Pos.XPosition < 0){
                Pos.XPosition = (Pos.XResolution / Pos.XCharSize - 1) * Pos.XCharSize;
                Pos.YPosition--;
                if (Pos.YPosition < 0)
                    Pos.YPosition = (Pos.YResolution / Pos.YCharSize - 1) * Pos.YCharSize;
            }
            //并调用putchar 函数打印空格符来覆盖之前的字符。
            putchar(Pos.FB_addr,Pos.XResolution,Pos.XPosition*Pos.XCharSize,Pos.YPosition*Pos.YCharSize,FRcolor,BKcolor,' ');
        }else if ((unsigned char)*(buf+count) == '\t') {
            line = ((Pos.XPosition + 8) & ~(8 - 1)) -Pos.XPosition; //8表示一个制表位占用8个显示字符
        Label_tab:
            line--;
            putchar(Pos.FB_addr,Pos.XResolution,Pos.XPosition*Pos.XCharSize,Pos.YPosition*Pos.YCharSize,FRcolor,BKcolor,' ');
            Pos.XPosition++;
        }else {
            putchar(Pos.FB_addr,Pos.XResolution,Pos.XPosition*Pos.XCharSize,Pos.YPosition*Pos.YCharSize,FRcolor,BKcolor,(unsigned char)*(buf+count));
            Pos.XPosition++;
        }

        if (Pos.XPosition >= (Pos.XResolution / Pos.XCharSize)) {
            Pos.YPosition++;
            Pos.XPosition = 0;
        if (Pos.YPosition >= (Pos.YResolution / Pos.YCharSize)) {
            Pos.YPosition = 0;
        }
    }
    return i;
}

void putchar(unsigned int *fb,int Xsize,int x,int y,unsigned int FRcolor,unsigned int BKcolor,unsigned char font) {
    int i = 0,j = 0;
    unsigned int *addr = NULL;
    unsigned char *fontp = NULL;
    int testval = 0;
    fontp = font_ascii[font];

    for (i = 0;i < 16;i++) {
        addr = fb + Xsize*(y + i) + x;
        testval = 0x100;
        for (j = 0;j < 8;j++) {
            testval = testval >> 1;
            if (*fontp &testval) 
                *addr = FRcolor;
            else
                *addr = BKcolor;
            addr++;
        }
        fontp++;
    }
}

int vsprintf(char * buf,const char *fmt, va_list args)
{
	char * str,*s;
	int flags;
	int field_width;
	int precision;
	int len,i;

	int qualifier;		/* 'h', 'l', 'L' or 'Z' for integer fields */

	for(str = buf; *fmt; fmt++)
	{

		if(*fmt != '%')
		{
			*str++ = *fmt;
			continue;
		}
		flags = 0;
		repeat:
			fmt++;
			switch(*fmt)
			{
				case '-':flags |= LEFT;	
				goto repeat;
				case '+':flags |= PLUS;	
				goto repeat;
				case ' ':flags |= SPACE;	
				goto repeat;
				case '#':flags |= SPECIAL;	
				goto repeat;
				case '0':flags |= ZEROPAD;	
				goto repeat;
			}

			/* get field width */

			field_width = -1;
			if(is_digit(*fmt))
				field_width = skip_atoi(&fmt);
			else if(*fmt == '*')
			{
				fmt++;
				field_width = va_arg(args, int);
				if(field_width < 0)
				{
					field_width = -field_width;
					flags |= LEFT;
				}
			}
			
			/* get the precision */

			precision = -1;
			if(*fmt == '.')
			{
				fmt++;
				if(is_digit(*fmt))
					precision = skip_atoi(&fmt);
				else if(*fmt == '*')
				{	
					fmt++;
					precision = va_arg(args, int);
				}
				if(precision < 0)
					precision = 0;
			}
			
			qualifier = -1;
			if(*fmt == 'h' || *fmt == 'l' || *fmt == 'L' || *fmt == 'Z')
			{	
				qualifier = *fmt;
				fmt++;
			}
							
			switch(*fmt)
			{
				case 'c':

					if(!(flags & LEFT))
						while(--field_width > 0)
							*str++ = ' ';
					*str++ = (unsigned char)va_arg(args, int);
					while(--field_width > 0)
						*str++ = ' ';
					break;

				case 's':
				
					s = va_arg(args,char *);
					if(!s)
						s = '\0';
					len = strlen(s);
					if(precision < 0)
						precision = len;
					else if(len > precision)
						len = precision;
					
					if(!(flags & LEFT))
						while(len < field_width--)
							*str++ = ' ';
					for(i = 0;i < len ;i++)
						*str++ = *s++;
					while(len < field_width--)
						*str++ = ' ';
					break;

				case 'o':
					
					if(qualifier == 'l')
						str = number(str,va_arg(args,unsigned long),8,field_width,precision,flags);
					else
						str = number(str,va_arg(args,unsigned int),8,field_width,precision,flags);
					break;

				case 'p':

					if(field_width == -1)
					{
						field_width = 2 * sizeof(void *);
						flags |= ZEROPAD;
					}

					str = number(str,(unsigned long)va_arg(args,void *),16,field_width,precision,flags);
					break;

				case 'x':

					flags |= SMALL;

				case 'X':

					if(qualifier == 'l')
						str = number(str,va_arg(args,unsigned long),16,field_width,precision,flags);
					else
						str = number(str,va_arg(args,unsigned int),16,field_width,precision,flags);
					break;

				case 'd':
				case 'i':

					flags |= SIGN;
				case 'u':

					if(qualifier == 'l')
						str = number(str,va_arg(args,unsigned long),10,field_width,precision,flags);
					else
						str = number(str,va_arg(args,unsigned int),10,field_width,precision,flags);
					break;

				case 'n':
					
					if(qualifier == 'l')
					{
						long *ip = va_arg(args,long *);
						*ip = (str - buf);
					}
					else
					{
						int *ip = va_arg(args,int *);
						*ip = (str - buf);
					}
					break;

				case '%':
					
					*str++ = '%';
					break;

				default:

					*str++ = '%';	
					if(*fmt)
						*str++ = *fmt;
					else
						fmt--;
					break;
			}

	}
	*str = '\0';
	return str - buf;
}