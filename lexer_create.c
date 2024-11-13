/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   lexer_create.c                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: zserobia <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/13 12:06:27 by zserobia          #+#    #+#             */
/*   Updated: 2024/11/13 12:06:30 by zserobia         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "parser.h"

// Crée un nouveau nœud lexer avec une valeur, un type de token et un index spécifié
t_lexer	*lexer_create(char *value, t_tokens token, int index)
{
	t_lexer	*new;
	
	new = (t_lexer *)malloc(sizeof(t_lexer));
	if (!new)
		return (NULL);
	new->str = value;
	new->token = token;
	new->index = index;
	new->next = NULL;
	new->prev = NULL;
	return (new);
}

// Ajoute un nouveau nœud lexer à la fin d'une liste chaînée
void	lexer_add_back(t_lexer **list, t_lexer *new_token)
{
	t_lexer	*temp;
	
	if (!*list)
	{
		*list = new_token;
		return ;
	}
	temp = *list;
	while (temp->next)
		temp = temp->next;
	temp->next = new_token;
	new_token->prev = temp;
}

// Crée et ajoute un nœud lexer à la liste
void	ft_create_lexer_list(char *value, t_tokens type, int index, t_lexer **list)
{
	t_lexer	*new_token;
	
	new_token = lexer_create(value, type, index);
	if (new_token)
		lexer_add_back(list, new_token);
}

// Lit un mot dans une ligne jusqu'à un espace ou un caractère spécial et retourne sa longueur
int	ft_read_word(char *line)
{
	int	i;
	
	i = 0;
	while (line[i] && !ft_ifspace(line[i]) && line[i] != '|' && line[i] != '>' && line[i] != '<')
		i++;
	return (i);
}
// Lit un mot entouré de guillemets et retourne sa longueur, y compris les guillemets fermants
int	ft_read_word_quote(char *line, char quote)
{
	int	i;
	
	i = 1;
	while (line[i] && line[i] != quote)
		i++;
	return (line[i] == quote) ? i + 1 : i;
}

// Analyse un token simple ou un mot entre guillemets, et le crée dans la liste lexer
int	ft_read_token_1(char *line, t_lexer **list, int index)
{
	int		len;
	char	quote;
	char	*word;
	
	if (*line == '\'' || *line == '"')
	{
		quote = *line;
		len = ft_read_word_quote(line, quote);
		word = ft_substr(line, 0, len);
		ft_create_lexer_list(word, WORD, index, list);
	}
	else
	{
		len = ft_read_word(line);
		word = ft_substr(line, 0, len);
		ft_create_lexer_list(word, WORD, index, list);
	}
	return (len);
}

// Fonction principale pour lire tous les tokens d'une ligne et les ajouter à la liste lexer
void	ft_read_token(char *line, t_lexer **list, t_shell *shell)
{
	int	index;
	int	len;
	
	index = 0;
	while (*line)
	{
		while (ft_ifspace(*line))
			line++;
		len = 1;
		if (*line == '|')
		{
			ft_create_lexer_list(strdup("|"), PIPE, index++, list);
			shell->count_pipe++;
		}
		else if (*line == '>' && *(line + 1) == '>')
		{
			ft_create_lexer_list(strdup(">>"), APPEND, index++, list);
			len = 2;
		}
		else if (*line == '>')
			ft_create_lexer_list(strdup(">"), OUT, index++, list);
		else if (*line == '<' && *(line + 1) == '<')
		{
			ft_create_lexer_list(strdup("<<"), HEREDOC, index++, list);
			len = 2;
		}
		else if (*line == '<')
			ft_create_lexer_list(strdup("<"), IN, index++, list);
		else
			len = ft_read_token_1(line, list, index++);
		line += len;
	}
}
