/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parser.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: zserobia <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/13 12:09:46 by zserobia          #+#    #+#             */
/*   Updated: 2024/11/13 12:09:48 by zserobia         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "parser.h"
// Supprime un nœud lexer donné dans la liste en fonction de l'index
void	delete_lexer(t_lexer **lexer, int index)
{
	t_lexer	*current;
	t_lexer	*prev;
	
	if (!lexer || !*lexer)
		return ;
	current = *lexer;
	prev = NULL;
	while (current && current->index != index)
	{
		prev = current;
		current = current->next;
	}
	if (!current)
		return ;
	if (prev)
		prev->next = current->next;
	else
		*lexer = current->next;
	if (current->next)
		current->next->prev = prev;
	if (current->str)
	{
		free(current->str);
		current->str = NULL;
	}
	free(current);
	current = NULL;
	printf("Lexer with index %d removed from lexer.\n", index);
}

// Ajoute une redirection en créant un nœud redirection et en supprimant les anciens nœuds lexer correspondants
int	add_new_redirection(t_lexer *current, t_lexer **lexer, t_pars_mini *pars_mini)
{
	t_lexer	*new;
	int		index1;
	int		index2;
	
	if (!current || !current->next)
		return (1);
	new = lexer_create(ft_strdup(current->next->str), current->token, pars_mini->num_redirections);  
	if (!new)
		return (1);
	lexer_add_back(&pars_mini->redirections, new);
	index1 = current->index;
	index2 = current->next->index;
	delete_lexer(lexer, index1);
	delete_lexer(lexer, index2);
	pars_mini->num_redirections++;
	return (0);
}

// Sépare les redirections des autres éléments dans lexer, ajoutant chaque redirection à pars_mini
int	separe_redirections(t_lexer **lexer, t_pars_mini *pars_mini)
{
	t_lexer	*current;
	
	current = *lexer;
	while (current && current->token != 1)
	{
		if (current->token > 1)
		{
			if (add_new_redirection(current, lexer, pars_mini))
				return (1);
			current = *lexer;
		}
		else
			current = current->next;
	}
	return (0);
}

// Compte le nombre de tokens de commande jusqu'au prochain PIPE
int	count_cmd(t_lexer *lexer)
{
	int	res;
	
	res = 0;
	while (lexer && lexer->token != PIPE)
	{
		++res;
		lexer = lexer->next;
	}
	return (res);
}

// Crée un tableau de chaînes représentant la commande, basée sur les tokens de lexer
char	**create_command(t_lexer *lexer)
{
	char	**cmd;
	int		i;
	int		len;
	
	if (!lexer)
		return (NULL);
	len = count_cmd(lexer);
	cmd = malloc(sizeof(char *) * (len + 1));
	if (!cmd)
		return (NULL);
	i = 0;
	while (i < len)
	{
		cmd[i] = ft_strdup(lexer->str);
		if (!cmd[i])
		{
			free_command(cmd);
			return (NULL);
		}
		lexer = lexer->next;
		i++;
	}
	cmd[i] = NULL;
	return (cmd);
}

// Crée un nouveau nœud de commande simple, en utilisant les redirections et la commande analysée
t_simple_cmds	*new_simple_cmd(t_lexer *lexer, t_pars_mini *pars_mini)
{
	t_simple_cmds	*new;
	
	new = malloc(sizeof(t_simple_cmds));
	if (!new)
		return (NULL);
	new->redirections = pars_mini->redirections;
	new->num_redirections = pars_mini->num_redirections;
	new->str = create_command(lexer);
	new->next = NULL;
	new->prev = NULL;
	return (new);
}

// Retourne le dernier nœud de la liste de commandes simples
static	t_simple_cmds *last_simple_cmd(t_simple_cmds *list)
{
	if (list)
	{
		while (list->next != NULL)
			list = list->next;
	}
	return (list);
}

// Ajoute une nouvelle commande simple à la fin de la liste de commandes
void	add_new_simple_cmd(t_simple_cmds **list, t_simple_cmds *new)
{
	t_simple_cmds	*last;
	
	if (*list)
	{
		last = last_simple_cmd(*list);
		last->next = new;
		new->prev = last;
	}
	else
		*list = new;
}
/*
// Function to handle redirection and initialize parser mini struct
int process_redirections_and_initialize(t_lexer **tmp, t_pars_mini *pars_mini, t_lexer **lexer_list, int *i) {
    pars_mini->redirections = NULL;
    pars_mini->num_redirections = 0;

    if (separe_redirections(tmp, pars_mini)) {
        printf("Error detaching redirections\n");
        return 1;  // Return error if detaching redirections failed
    }

    if (!(*i)++) {
        *lexer_list = *tmp;
    } else {
        *tmp = (*tmp)->next;
    }

    return 0;  // Return 0 to indicate successful processing
}

// Function to create and add new commands to the list
int create_and_add_commands(int *count_pipe, t_lexer **tmp, t_simple_cmds **commands) {
    t_simple_cmds *new_cmd;

    while (*count_pipe > 0 && *tmp) {
        new_cmd = new_simple_cmd(*tmp, NULL);
        if (!new_cmd) {
            printf("Error creating new command node\n");
            return 1;
        }
        add_new_simple_cmd(commands, new_cmd);

        if (!(*tmp)) {
            return 1;
        }
        if (--(*count_pipe) <= 0) {
            return 1;
        }

        while (*tmp && (*tmp)->token != PIPE) {
            *tmp = (*tmp)->next;
        }
    }

    return 0;
}

// Main parser function to process commands and redirections
void parser_part(int count_pipe, t_lexer *lexer_list) {
    t_lexer *tmp = lexer_list;
    t_pars_mini pars_mini;
    t_simple_cmds *commands = NULL;
    int i = 0;

    count_pipe += 1;

    while (count_pipe > 0 && tmp) {
        // Process redirections and initialize
        if (process_redirections_and_initialize(&tmp, &pars_mini, &lexer_list, &i) != 0) {
            break;
        }

        // Create and add new commands
        if (create_and_add_commands(&count_pipe, &tmp, &commands) != 0) {
            break;
        }
    }

    // Print commands and clean up memory
    print_simple_cmds(commands);
    free_lexer_list(&lexer_list);
    free_simple_cmds_list(commands);
}
*/

void	parser_part(int count_pipe, t_lexer *lexer_list)
{
	t_lexer			*tmp;
	t_pars_mini		pars_mini;
	t_simple_cmds	*new_cmd;
	t_simple_cmds	*commands;
	int				i;
	
	i = 0;
	tmp = lexer_list;
	commands = NULL;
	count_pipe = count_pipe + 1;
	while (count_pipe > 0 && tmp)
	{
		pars_mini.redirections = NULL;
		pars_mini.num_redirections = 0;
		if (separe_redirections(&tmp, &pars_mini))
		{
			printf("Error detaching redirections\n");
			break;
		}
		if (!i++)
			lexer_list = tmp;
		else
			tmp = tmp->next;
		new_cmd = new_simple_cmd(tmp, &pars_mini);
		if (!new_cmd)
		{
			printf("Error creating new command node\n");
			break;
		}
		add_new_simple_cmd(&commands, new_cmd);
		if (!tmp)
			break;
		if (--count_pipe <= 0)
			break;
		while (tmp && tmp->token != PIPE)
			tmp = tmp->next;
	}
	print_simple_cmds(commands);
	free_lexer_list(&lexer_list);
	lexer_list = NULL;
	free_simple_cmds_list(commands);
}