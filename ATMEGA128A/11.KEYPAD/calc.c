/*
 * calc.c
 *
 * Created: 2026-06-29 오후 4:09:34
 *  Author: kccistc
 */ 

extern uint8_t delete_queue(void);

void calc(void);

int operator = '+';
int buff[10][100];
int num[10];

int num_idx = 0;
int buff_idx = 0;

void calc()
{
	if(delete_queue() == '=')
	{
		while(!queue_empty())
		{
			int data = delete_queue();
			if(data == '+' || data == '-' || data == '*' || data == '/')
			{
				operator = data;
				num_idx++;
			}
			else
			{
				buff[num_idx][buff_idx++] = data;
			}
		}
		for(int i = 0;i<num_idx;i++)
		{
			atoi(num)
		}
		
	}
}