#include "lexer.h"

void	exec_pipe(t_ast *tree)
{
	int pid;
	int pp[2];
	int	fd;
	int status;

	if (pipe(pp) == -1)
		return ;
//printf("PIPE type=%d left=%d right=%d\n", tree->type, tree->left->type, tree->right->type);
	pid = fork();
	if (!pid)
	{
		close(pp[0]);
		fd = dup2(pp[1], 1);
		exec_red(tree->left, 0);
	}
	else
	{
		close(pp[1]);
		fd = dup2(pp[0], 0);
		waitpid(pid, &status, 0);
		if (tree->right->type == NODE_PIPE)
			exec_pipe(tree->right);
		else
			exec_red(tree->right, 0);
	}
}

void	exec_red(t_ast *tree, int i)
{
	int fd;
	int new;
	char *file;

	if (tree->type == NODE_CMDPATH)
	{
//	printf("RED type=%d data=%s\n", tree->type, tree->data);
		get_arg(tree->right, &(g_my.arg));
		exec_cmd(tree);
	}
	if (!i)
	{
		get_arg(tree->left->right, &(g_my.arg));
		g_my.exec = tree->left;
	}
	if (tree->right->type == NODE_CMDPATH)
	{
		get_arg(tree->right->right, &(g_my.arg));
		file = tree->right->data;
	}
	else
	{
		get_arg(tree->right->left->right, &(g_my.arg));
		file = tree->right->left->data;
	}
	if (tree->type == NODE_GREATER)
		fd = open(file, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
	else if (tree->type == NODE_DGREATER)
		fd = open(file, O_WRONLY | O_CREAT | O_APPEND, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
	if (tree->type == NODE_SMALLER)
	{
		fd = open(file, O_RDONLY, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
		if ((new = dup2(fd, 0)) == -1)
			printf("FD=%d ERROR\n", fd);
	}
	else
	{
		if ((new = dup2(fd, 1)) == -1)
			printf("FD=%d ERROR\n", fd);
	}
	if (tree->right->type == NODE_CMDPATH)
	{
		exec_cmd(g_my.exec);
	}
	else
		exec_red(tree->right, 1);
}

void	exec_tree(t_ast *tree)
{
	int		pid;
	int		fd;

	g_my.arg = 0;
	pid = fork();
	if (!pid)
	{
//		printf("TREE type=%d left=%p p_right=%p\n", tree->type, tree->left, tree->right, tree->data);
		if (tree->type == NODE_PIPE)
			exec_pipe(tree);
		else if (tree->type == NODE_CMDPATH)
		{
			get_arg(tree->right, &(g_my.arg));
			exec_cmd(tree);
		}
		else
			exec_red(tree, 0);
	}
	wait(NULL);

}

static char *get_path_spec(char *str)
{
	char	**tab;
	char	*path;

	tab = get_path_table_y(get_env(g_my.env_list, "PATH"));  // FREEEEE
	path = get_abs_path_y(tab, str);

		return (path);
}

void	exec_cmd(t_ast *tree)
{
	char	*path;
	char	**tab;
	int 	pid;
	t_list	*new;

	path = get_path_spec(tree->data);
	if (!(new = ft_lstnew(path)))
		printf("(1)\n");
	ft_lstadd_front(&(g_my.arg), new);
	if (!(tab = list_to_tab(g_my.arg)))
		printf("(2)\n");
/*	for (int i=0; tab[i]; i++)
		printf("tab[%d]=%s\n", i, tab[i]);
*/	execve(tab[0], tab, NULL);
	printf("OOOUUUIIII\n");
}


void	get_arg(t_ast *tree, t_list **lst)
{
	t_list	*new;

	while (tree)
	{
		new = ft_lstnew(ft_strdup(tree->data));
		ft_lstadd_back(lst, new);
		tree = tree->right;
	}
}

char	**list_to_tab(t_list *lst)
{
	char	**tab;
	int		size;
	int		j;
	t_list	*save;

	j = -1;
	save = lst;
	size = ft_lstsize(lst);
	tab = (char**)malloc(sizeof(char*) * (size + 1));
	while (lst)
	{
		tab[++j] = ft_strdup((char*)(lst->content));
		lst = lst->next;
	}
	tab[++j] = NULL;
	ft_lstclear(&save, free);
	return (tab);
}
