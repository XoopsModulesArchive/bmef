<?php

# $Id: respondents.inc,v 1.4 2003/01/30 20:14:53 jimmerman Exp $

// Written by James Flemer
// <jflemer@alum.rpi.edu>
// <jflemer@acm.rpi.edu>

	$sql = "SELECT username, fname, lname, realm, disabled, expiration FROM ".TABLE_RESPONDENT;

	/* load only accounts available to _this_ user */
	if($HTTP_SESSION_VARS['acl']['superuser'] != 'Y') {
		$realms = array_to_insql(
			array_intersect(
				$HTTP_SESSION_VARS['acl']['pall'],
				$HTTP_SESSION_VARS['acl']['puser']));
		$sql .= " WHERE realm $realms";
	}

	$sql .= ' ORDER BY ';
	$base = $GLOBALS['ESPCONFIG']['ME'] .'?where=respondents';
	if(!isset($HTTP_GET_VARS['u'])) $HTTP_GET_VARS['u'] = '';
	if(!isset($HTTP_GET_VARS['g'])) $HTTP_GET_VARS['g'] = '';
	if(!isset($HTTP_GET_VARS['f'])) $HTTP_GET_VARS['f'] = '';
	if(!isset($HTTP_GET_VARS['l'])) $HTTP_GET_VARS['l'] = '';
	if(!isset($HTTP_GET_VARS['s'])) $HTTP_GET_VARS['s'] = '';

	if($HTTP_GET_VARS['u'] == 'd') {
		$us = 'username DESC';
		$u = 'a';
	} else {
		$HTTP_GET_VARS['u'] = 'a';
		$us = 'username ASC';
		$u = 'd';
	}
	if($HTTP_GET_VARS['g'] == 'd') {
		$gs = 'realm DESC';
		$g = 'a';
	} else {
		$HTTP_GET_VARS['g'] = 'a';
		$gs = 'realm ASC';
		$g = 'd';
	}
	if($HTTP_GET_VARS['f'] == 'd' || $HTTP_GET_VARS['l'] == 'd') {
		$ls = 'lname DESC';
		$l = 'a';
		$fs = 'fname DESC';
		$f = 'a';
	} else {
		$HTTP_GET_VARS['l'] = 'a';
		$ls = 'lname ASC';
		$l = 'd';
		$HTTP_GET_VARS['f'] = 'a';
		$fs = 'fname ASC';
		$f = 'd';
	}
	if($HTTP_GET_VARS['s'] == 'g') {
		$sql .= "$gs, $us";
		$u = "&s=u&g=" . $HTTP_GET_VARS['g'] ."&u=". $HTTP_GET_VARS['u'];
		$g = "&s=g&g=$g&u=". $HTTP_GET_VARS['u'];
		$f = "&s=f";
		$l = "&s=l";
	} elseif($HTTP_GET_VARS['s'] == 'f' || $HTTP_GET_VARS['s'] == 'l') {
		$sql .= "$fs, $ls";
		$f = "&s=f&f=$f";
		$l = "&s=l&l=$l";
		$u = "&s=u";
		$g = "&s=g";
	} else {
		$sql .= "$us, $gs";
		$u = "&s=u&g=" . $HTTP_GET_VARS['g'] ."&u=$u";
		$g = "&s=g&g=" . $HTTP_GET_VARS['g'] ."&u=". $HTTP_GET_VARS['u'];
		$f = "&s=f";
		$l = "&s=l";
	}
	$result = mysql_query($sql);
	
	$bg = $ESPCONFIG['bgalt_color2'];
?>
<h2><?php mb_echo('Manage Respondent Accounts'); ?></h2>
<p><?php mb_echo('Click on a username to edit, or click on add new user below.'); ?></p>
<table border="0" cellspacing="0" cellpadding="4" align="center" bgcolor="<?php echo($ESPCONFIG['active_bgcolor']); ?>" width="95%">
	<tr bgcolor="<?php echo($bg); ?>">
		<th align="left"><a href="<?php echo($base . $u); ?>"><?php mb_echo('Username'); ?></a></th>
		<th align="left"><a href="<?php echo($base . $f); ?>"><?php mb_echo('First Name'); ?></a></th>
		<th align="left"><a href="<?php echo($base . $l); ?>"><?php mb_echo('Last Name'); ?></a></th>
		<th align="left"><a href="<?php echo($base . $g); ?>"><?php mb_echo('Group'); ?></a></th>
		<th align="left">&nbsp;</th>
	</tr>
<?php
	while(list($u, $fname, $lname, $r, $d, $e) = mysql_fetch_row($result)) {
		if($d == 'N')
			$d = '&nbsp;';
		else
			$d = '('. mb_('disabled') .')';
		
		if (empty($fname)) $fname = '&nbsp;';
		if (empty($lname)) $lname = '&nbsp;';

		if ($bg == $ESPCONFIG['bgalt_color1'])
			$bg =& $ESPCONFIG['bgalt_color2'];
		else
			$bg =& $ESPCONFIG['bgalt_color1'];
?>
	<tr bgcolor="<?php echo($bg); ?>">
		<td>
			<a href="<?php echo($GLOBALS['ESPCONFIG']['ME'] ."?where=admrespondent&u=$u&r=$r"); ?>"><?php echo($u); ?></a>
		</td>
		<td><?php echo($fname); ?></td>
		<td><?php echo($lname); ?></td>
		<td><?php echo($r); ?></td>
		<td><?php echo($d); ?></td>
	</tr>
<?php
	}
	if ($bg == $ESPCONFIG['bgalt_color1'])
		$bg =& $ESPCONFIG['bgalt_color2'];
	else
		$bg =& $ESPCONFIG['bgalt_color1'];
?>
	<tr bgcolor="<?php echo($bg); ?>">
		<th colspan="5" align="center">
			<table border="0" cellpadding="4">
				<tr>
					<th>
					<a href="<?php echo($GLOBALS['ESPCONFIG']['ME'] ."?where=admrespondent"); ?>">
						<?php mb_echo('Add a new Respondent'); ?>
					</a>
					</th>
					<th>
					<a href="<?php echo($GLOBALS['ESPCONFIG']['ME'] ."?where=upload&account_type=respondents"); ?>">
						<?php mb_echo('Bulk Upload Respondents'); ?>
					</a>
					</th>
				</tr>
			</table>
		</th>
	</tr>
</table>
<?php echo("<a href=\"". $GLOBALS['ESPCONFIG']['ME'] ."?where=manage\">" . mb_('Go back to Management Interface') . "</a>\n"); ?>