# provided by hlu, modified by obz
NAME = mouse
OBJS = mouse.obj
CC = cl
RM = del
CFLAGS = -Zi -c
LDFLAGS = -Zi

.c.o:
	$(CC) $(CFLAGS) $<

$(NAME) : $(OBJS)
	$(CC) $(LDFLAGS) -Fe$(NAME) $(OBJS) $(LIBS)

clean:
	-$(RM) $(NAME) $(OBJS)

