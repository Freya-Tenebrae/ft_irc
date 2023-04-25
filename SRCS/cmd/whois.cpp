/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   whois.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: plam <plam@student.42.fr>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/01/16 18:15:54 by cmaginot          #+#    #+#             */
/*   Updated: 2023/04/25 16:53:36 by plam             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../INCL/User.hpp"
#include "../../INCL/Reply.hpp"
#include "../../INCL/Server.hpp"

/*
Command: WHOIS
Parameters: [<target>] <nick>

This command is used to query information about particular users. The server will answer
this command with several numeric messages with information about the nicks, ending with
RPL_ENDOFWHOIS.

Servers MUST end their reply to WHOIS messages with one of these numerics:

ERR_NOSUCHNICK (401)
ERR_NOSUCHSERVER (402)
ERR_NONICKNAMEGIVEN (431)
RPL_ENDOFWHOIS (318)otherwise, even if they did not send any other numeric message. This allows
clients to stop waiting for new numerics.
In exceptional error conditions, servers MAY not reply to a WHOIS command. Clients SHOULD
implement a hard timeout to avoid waiting for a reply which won’t come.

Client MUST NOT not assume all numeric messages are sent at once, as server can interleave
other messages before the end of the WHOIS response.

If the <target> parameter is specified, it SHOULD be a server name or the nick of a user->
Servers SHOULD send the query to a specific server with that name, or to the server <target>
is connected to, respectively. Typically, it is used by clients who want to know how long
the user in question has been idle (as typically only the server the user is directly connected
to knows that information, while everything else this command returns is globally known).

The following numerics MAY be returned as part of the whois reply:

RPL_WHOISCERTFP (276)
RPL_WHOISREGNICK (307)
RPL_WHOISUSER (311)
RPL_WHOISSERVER (312)
RPL_WHOISOPERATOR (313)
RPL_WHOISIDLE (317)
RPL_WHOISCHANNELS (319)
RPL_WHOISSPECIAL (320)
RPL_WHOISACCOUNT (330)
RPL_WHOISACTUALLY (338)
RPL_WHOISHOST (378)
RPL_WHOISMODES (379)
RPL_WHOISSECURE (671)
RPL_AWAY (301)

Servers typically send some of these numerics only to the client itself and to servers operators,
as they contain privacy-sensitive information that should not be revealed to other users.

Server implementers wishing to send information not covered by these numerics may send other
vendor-specific numerics, such that:

the first and second parameters MUST be the client’s nick, and the target nick, and
the last parameter SHOULD be designed to be human-readable, so that user interfaces can display
unknown numerics
Additionally, server implementers should consider submitting these to IRCv3 for standardization,
if relevant.

Optional extensions
This section describes extension to the common WHOIS command above. They exist mainly on
historical ervers, and are rarely implemented, because of resource usage they incur.

Servers MAY allow more than one target nick. They can advertise the maximum the number of
target users per WHOIS command via the TARGMAX RPL_ISUPPORT parameter, and silently drop
targets if the number of targets exceeds the limit.

Servers MAY allow wildcards in <nick>. Servers who do SHOULD reply with information about all
matching nicks. They may restrict what information is available in this case, to limit resource
usage.

IRCv3 multi-prefix Extension

Examples :

Command Examples:

  WHOIS val                     ; request information on user "val"
  WHOIS val val                 ; request information on user "val",
                                from the server they are on
  WHOIS calcium.libera.chat val ; request information on user "val",
                                from server calcium.libera.chat

Reply Example:

  :calcium.libera.chat 311 val val ~val limnoria/val * :Val
  :calcium.libera.chat 319 val val :#ircv3 #libera +#limnoria
  :calcium.libera.chat 319 val val :#weechat
  :calcium.libera.chat 312 val val calcium.libera.chat :Montreal, CA
  :calcium.libera.chat 671 val val :is using a secure connection [TLSv1.3, TLS_AES_256_GCM_SHA384]
  :calcium.libera.chat 317 val val 657 1628028154 :seconds idle, signon time
  :calcium.libera.chat 330 val val pinkieval :is logged in as
  :calcium.libera.chat 318 val val :End of /WHOIS list.
*/

std::vector<Reply>	Server::whois(User *user, std::vector<std::string> args)
{
	std::vector<Reply> reply;
	(void)user;							// get rid of this to make it work
	(void)args;							// get rid of this to make it work
	if (args.size() == 1)
	{
		if (args[0].empty() == false)
		{
			for (std::vector<User *>::iterator it_usr = _usr_list.begin(); it_usr != _usr_list.end(); it_usr++)
			{
				if ((*it_usr)->get_nickname().compare(args[0]))
				{
					reply.push_back(RPL_WHOISUSER);
					reply[reply.size()-1].add_arg("channel to do", "channel");
					reply[reply.size()-1].add_arg((*it_usr)->get_nickname(), "is logged in as");
				}
				break ;
			}
		}
		else
			reply.push_back(ERR_NONICKNAMEGIVEN);
	}
	else if (args.size() == 2) 
	{
		if (args[0].empty() == false && (args[0].compare(this->get_name()) || args[0] == args[1]))
		{
			for (std::vector<User *>::iterator it_usr = _usr_list.begin(); it_usr != _usr_list.end(); it_usr++)
			{
				if ((*it_usr)->get_nickname().compare(args[0]))
				{
					reply.push_back(RPL_WHOISUSER);
					reply[reply.size()-1].add_arg((*it_usr)->get_realname(), "is logged in as");
				}
			}
			if (user->check_if_mode_is_used('o') == true)
			{
				reply.push_back(RPL_WHOISCERTFP);
				reply.push_back(RPL_WHOISREGNICK);
				reply.push_back(RPL_WHOISSERVER);
				reply.push_back(RPL_WHOISOPERATOR);
				reply.push_back(RPL_WHOISCHANNELS);
				if (args[0][0] == '#')
				{
					args[0].erase(0, 1);
					for (std::vector<Channel *>::iterator it = _cha_list.begin(); it != _cha_list.end(); it++)
					{
						if ((*it)->get_name().compare(args[0]))
							reply[reply.size()-1].add_arg((*it)->get_name(), "channel");
					}
				}
				else if (args[0] == args[1])
				{
					for (std::vector<Channel *>::iterator it = _cha_list.begin(); it != _cha_list.end(); it++)
					{
						const std::vector<User *> ch_usr_list_ref = (*it)->get_ch_usr_list();
						for (std::vector<User *>::const_iterator it_usr = ch_usr_list_ref.begin(); it_usr != ch_usr_list_ref.end(); it_usr++)
							if ((*it_usr)->get_nickname() == args[1])
								reply[reply.size()-1].add_arg((*it)->get_name(), " ");
					}
				}
				//reply.push_back(RPL_WHOISSPECIAL);
				reply.push_back(RPL_WHOISACCOUNT);
				reply.push_back(RPL_WHOISACTUALLY);
				reply.push_back(RPL_WHOISHOST);
				reply.push_back(RPL_WHOISMODES);
				reply.push_back(RPL_WHOISSECURE);
				reply.push_back(RPL_WHOISIDLE);
				reply.push_back(RPL_AWAY);
			}
		}
	}
	else
		reply.push_back(ERR_NOSUCHSERVER);
	reply.push_back(RPL_ENDOFWHOIS);
	for (std::vector<Reply>::iterator it = reply.begin(); it != reply.end(); it++)
	{
		it->add_user(user);
		it->prep_to_send(1);
	}
	return (reply);
}
