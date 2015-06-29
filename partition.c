#include "command.h"
void lineError(int numLine)
{
  fprintf(stderr, "%d:\n", numLine);
  exit(1);
}

int isRegular(char input)
//detect if the charactor is a regular charactor
{
  if (input >= 48 && input <= 57)
    return 1;
  if (input >= 65 && input <= 90)
    return 1;
  if (input >= 97 && input <= 122)
    return 1;
  switch(input)
    {
    case '!':
    case '%':
    case '+':
    case ',':
    case '-':
    case '.':
    case '/':
    case ':':
    case '@':
    case '^':
    case '_':
	  return 1;
    default:
      return 0;
    }
}



queue partition(char* input)
{
  char* current = input;
  int Status_comment = 0, Status_backslash = 0;
  int len = 0, numLine = 1;

  char* tmpStr = (char*)checked_malloc(3*strlen(input)*sizeof(char));
  while(*current != '\0')
    {
      if(Status_comment && *current != '\n')
	{
	  current++;
	  continue;
	}

      switch(*current)
        {
	case '#':
	  if(isRegular(*(current + 1)))
	    strcat(tmpStr, "#");
	  else
	    Status_comment = 1;
	  break;

	case '\\':
	  if(!Status_comment)
	    Status_backslash = 1;
	  if(*(current + 1) != '\n')
	    {
	      lineError(numLine);
	    }
	  break;

	case '\n':
	  numLine++;
	  if(Status_backslash)
	    {
	      Status_backslash = 0;
	      strcat(tmpStr, " \\ ");
	      break;
	    }
	  else if(Status_comment)
	    {
	      Status_comment = 0;
	      strcat(tmpStr, " \n ");
	      break;
	    }
	  else
	    {
	      strcat(tmpStr, " \n ");
	    }
	  break;

	case '\t':
	case ' ':
	  strcat(tmpStr, " ");
	  break;

	case ';':
	  strcat(tmpStr, " ; ");
	  break;

	case '>':
	  strcat(tmpStr, " > ");
	  len = len + 3;
	  break;

	case '<':
	  strcat(tmpStr, " < ");
	  len = len + 3;
	  break;

	case '(':
	  strcat(tmpStr, " ( ");
	  len = len + 3;
	  break;

	case ')':
	  strcat(tmpStr, " ) ");
	  break;

	case '|':
	  if(*(current + 1) == '|')
	    {
	      strcat(tmpStr, " || ");
	      current++;
	    }
	  else
	    {
	      strcat(tmpStr, " | ");
	    }
	  break;

	case '&':
	  if(*(current + 1) == '&')
	    {
	      strcat(tmpStr, " && ");
	      len = len + 4;
	      current++;
	    }
	  else
	    lineError(numLine);
	  break;

	default:
	  if(isRegular(*current))
	    strncat(tmpStr, current, 1);
	  else
	    lineError(numLine);
        }

      current++;
    }

  /* testing code */
  /*
  printf("%s\n", input);
  printf("%s\n", tmpStr);
  */

  //partition and put them in queue
  queue tokenQueue = q_empty();
  char* pch;
  token_type key;
  pch = strtok(tmpStr, " ");
  while(pch != NULL)
    {
      //determine token type
      if(*pch == '<')
		key = IN;
      else if(*pch == '>')
		key = OUT;
      else if(*pch == '&')
		key = AND;
      else if(*pch == '|') 
	  {
		if(strlen(pch) == 2)
		  key = OR;
		else
		  key = PIPE;
	  }
      else if(*pch == '\n')
		key = NEW_LINE;
	  else if(*pch == ';')
		key = SEQ;
      else if(*pch == '\\')
		key = BACK_SLASH;
      else if(*pch == '(')
		key = LB;
      else if(*pch == ')')
		key = RB;
      else
		key = WORD;

      //construct the pair
      pair* inputPair = b_pair(key, pch);

      //enqueue the pair
      tokenQueue = enqueue(inputPair, tokenQueue);

      //testing
      // printf("%s\n", pch);

      pch = strtok(NULL, " ");
    }

  //free(tmpStr);

  return tokenQueue;
}

