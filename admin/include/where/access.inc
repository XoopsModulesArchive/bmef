<?php

# $Id: access.inc,v 1.8 2003/03/05 19:26:40 jimmerman Exp $

// Written by James Flemer
// For eGrad2000.com
// <jflemer@alum.rpi.edu>

	if (!empty($HTTP_POST_VARS['sid']))
		$sid = intval($HTTP_POST_VARS['sid']);
	elseif (!empty($HTTP_GET_VARS['sid']))
		$sid = intval($HTTP_GET_VARS['sid']);
	else
		$sid = '';

	$errstr = "";
	$bg = '';

	if ($sid) {
		if($xoopsUser->isAdmin()) {
			$sql = "SELECT s.name, s.title, s.owner, s.realm, s.public
				FROM ".TABLE_SURVEY." s WHERE s.id = '$sid'";
			$sql1 = "SELECT a.realm, a.maxlogin, a.resume, a.navigate FROM ".TABLE_ACCESS." a
				WHERE a.survey_id = '$sid' ORDER BY a.realm";
		} else {
			$realms = array_to_insql(
				array_intersect(
					$HTTP_SESSION_VARS['acl']['pall'],
					array_merge(
						$HTTP_SESSION_VARS['acl']['pall'],
						$HTTP_SESSION_VARS['acl']['pdesign'])));
			$sql = "SELECT s.name, s.title, s.owner, s.realm, s.public
				FROM ".TABLE_SURVEY." s WHERE s.id = '$sid' AND NOT (status & " .STATUS_DELETED. ") AND (s.owner = '".
				$xoopsUser->uid() ."' || s.realm $realms)";
			$sql1 = "SELECT a.realm, a.maxlogin, a.resume, a.navigate FROM ".TABLE_ACCESS." a, survey s
				WHERE a.survey_id = '$sid' AND s.id=a.survey_id AND (s.owner = '".
				$xoopsUser->uid() ."' || s.realm $realms) ORDER BY a.realm";
		}
		$result = mysql_query($sql);
		if (mysql_num_rows($result) < 1) {
			$sid = 0;
		}
	}
	if ($sid) {
		list($name,$title,$owner,$realm,$public) = mysql_fetch_row($result);
		mysql_free_result($result);

		if (!empty($HTTP_POST_VARS['op']))
			$op = $HTTP_POST_VARS['op'];
		elseif (!empty($HTTP_GET_VARS['op']))
			$op = $HTTP_GET_VARS['op'];
		else
			$op = '';

		if (!empty($HTTP_POST_VARS['realm']))
			$arealm = _addslashes($HTTP_POST_VARS['realm']);
		elseif (!empty($HTTP_GET_VARS['realm']))
			$arealm = _addslashes($HTTP_GET_VARS['realm']);

		if (isset($HTTP_POST_VARS['resume']))
			$resume = 'Y';
		else
			$resume = 'N';
		
		if (isset($HTTP_POST_VARS['navigate']))
			$navigate = 'Y';
		else
			$navigate = 'N';
		
		if (!empty($HTTP_POST_VARS['max']))
			$max = intval($HTTP_POST_VARS['max']);
		elseif (!empty($HTTP_GET_VARS['max']))
			$max = intval($HTTP_GET_VARS['max']);
		else
			$max = 0;

		if ($op == 'a') {
			if (empty($HTTP_POST_VARS['realm'])) {
				$errstr = mkerror(_('Please select a group.'));
			} else {
				$sql = "INSERT INTO access (survey_id, realm, maxlogin, resume, navigate)
						VALUES ('$sid', '$arealm', '$max', '$resume', '$navigate')";
				mysql_query($sql);
			}
		} elseif ($op == 'r') {
			$sql = "DELETE FROM ".TABLE_ACCESS." WHERE survey_id = '$sid' AND realm = '$arealm'";
			mysql_query($sql);
		} elseif ($op == 'v') {
			$sql = "UPDATE survey SET public = 'N' WHERE id = '$sid'";
			mysql_query($sql);
			$sid = 0;
		} elseif ($op == 'p') {
			$sql = "UPDATE survey SET public = 'Y' WHERE id = '$sid'";
			mysql_query($sql);
			$sid = 0;
		}
	}
	if ($sid) {
		if ($public == 'N')
			$public = mb_('Private');
		else
			$public = mb_('Public');

		$r = '<select name="realm"><option></option>';
		$groups = array();
		if($xoopsUser->isAdmin()) {
			$sql = "SELECT name FROM ".TABLE_REALM."";
			$result = mysql_query($sql);
			while( list($g) = mysql_fetch_row($result) ) {
				array_push($groups, $g);
			}
			mysql_free_result($result);
		} else {
			$groups =& $HTTP_SESSION_VARS['acl']['pgroup'];
		}
		foreach($groups as $g) {
			$r .= "<option value=\"$g\">$g</option>";
		}
		$r .= '</select>';
	}
?>
<h2><?php mb_echo('Survey Access'); ?></h2>

<?php if(!empty($errstr)) echo("<p>$errstr</p>\n"); ?>

<div align="left">
<p><?php
	mb_echo('This lets you control who has access to fill out a form.
Public surveys let anyone submit data.
Private surveys are restricted by Respondent Groups.'); ?></p>

<p><b><?php mb_echo('Note'); ?>:</b>
<?php printf(_('You must use %s when using private surveys.'),
	'<tt>'. substr(strrchr($ESPCONFIG['handler_prefix'], '/'), 1) .
	' &amp; '. substr(strrchr($ESPCONFIG['handler'], '/'), 1) . '</tt>'); ?></p>
</div>

<?php echo("<a href=\"". $GLOBALS['ESPCONFIG']['ME'] ."?where=manage\">" . mb_('Go back to Management Interface') . "</a>\n"); ?>
<?php if ($sid) { ?>
<input type="hidden" name="where" value="access">
<input type="hidden" name="sid" value="<?php echo($sid); ?>">
<input type="hidden" name="op" value="a">
<table border="0" align="center" cellspacing="0" cellpadding="4" bgcolor="<?php echo($ESPCONFIG['active_bgcolor']); ?>" width="95%">
	<tr bgcolor="#dddddd"><th align="left"><?php mb_echo('ID'); ?></th><td colspan="5"><?php echo($sid); ?></td></tr>
	<tr bgcolor="#dddddd"><th align="left"><?php mb_echo('Name'); ?></th><td colspan="5"><?php echo($name); ?></tr>
	<tr bgcolor="#dddddd"><th align="left"><?php mb_echo('Title'); ?></th><td colspan="5"><?php echo($title); ?></tr>
	<tr bgcolor="#dddddd"><th align="left"><?php mb_echo('Owner'); ?></th><td colspan="5"><?php echo($owner); ?></tr>
	<tr bgcolor="#dddddd"><th align="left"><?php mb_echo('Group'); ?></th><td colspan="5"><?php echo($realm); ?></tr>
	<tr bgcolor="#dddddd"><th align="left"><?php mb_echo('Public'); ?></th><td colspan="5"><?php echo($public); ?></tr>
	<tr bgcolor="#dddddd"><td colspan="6"><hr></td></tr>
	<tr><th align="left"><?php mb_echo('Group'); ?></th>
		<th align="left"><?php mb_echo('Max Responses'); ?></th>
		<th align="left"><?php mb_echo('Save/Restore'); ?></th>
		<th align="left"><?php mb_echo('Back/Forward'); ?></th>
		<td>&nbsp;</td></tr>
<?php
		$result = mysql_query($sql1);
		while ( list($arealm, $amax, $aresume, $anavigate) = mysql_fetch_row($result) ) { ?>
	<tr><td><?php echo($arealm); ?></td>
		<td><?php echo($amax); ?></td>
		<td><?php echo(($aresume == 'Y') ? mb_('Yes') : mb_('No')); ?></td>
		<td><?php echo(($anavigate == 'Y') ? mb_('Yes') : mb_('No')); ?></td>
		<td><a href="<?php echo($GLOBALS['ESPCONFIG']['ME'] ."?where=access&sid=$sid&op=r&realm=" . urlencode($arealm)); ?>">
		<?php mb_echo('Remove'); ?></a></td></tr>
<?php	} ?>
	<tr><td><?php echo($r); ?></td>
		<td><input type="text" name="max" size="5"></td>
		<td><input type="checkbox" name="resume" /></td>
		<td><input type="checkbox" name="navigate" /></td>
		<td><input type="submit" value="<?php mb_echo('Add'); ?>"></td></tr>
</table>
<?php echo("<a href=\"". $GLOBALS['ESPCONFIG']['ME'] ."?where=manage\">" . mb_('Go back to Management Interface') . "</a>\n"); ?>
<?php
		return;
	}
?>
<table border="0" align="center" cellspacing="0" cellpadding="4" bgcolor="<?php echo($ESPCONFIG['active_bgcolor']); ?>" width="95%">
	<tr bgcolor="#dddddd">
		<th align="left"><?php mb_echo('ID'); ?></th>
		<th align="left"><?php mb_echo('Name'); ?></th>
		<th align="left"><?php mb_echo('Title'); ?></th>
		<th align="left"><?php mb_echo('Owner'); ?></th>
		<th align="left"><?php mb_echo('Group'); ?></th>
		<th align="left"><?php mb_echo('Public'); ?></th>
		<td>&nbsp;</td>
	</tr>
<?php
	/* load names and titles of all surveys available to
	 * _this_ user */
	if($xoopsUser->isAdmin()) {
		$sql = "SELECT id,name,title,owner,realm,public FROM ".TABLE_SURVEY." WHERE NOT (status & " .STATUS_DELETED. ") ORDER BY id DESC";
	} else {
		$realms = array_to_insql(
			array_intersect(
				$HTTP_SESSION_VARS['acl']['pall'],
				array_merge(
					$HTTP_SESSION_VARS['acl']['pall'],
					$HTTP_SESSION_VARS['acl']['pdesign'])));
		$sql = "SELECT id,name,title,owner,realm,public
			FROM ".TABLE_SURVEY." WHERE NOT (status & " .STATUS_DELETED. ") AND (owner = '".
			$xoopsUser->uid() ."' || realm $realms) ORDER BY id DESC";
	}
	$result = mysql_query($sql);

	while(list($sid,$name,$title,$owner,$realm,$public) = mysql_fetch_row($result)) {
		if($bg != $ESPCONFIG['bgalt_color1'])
			$bg = $ESPCONFIG['bgalt_color1'];
		else
			$bg = $ESPCONFIG['bgalt_color2'];

		if ($public == 'N') {
			$public = mb_('Private');
			$op     = '<a href="'. $GLOBALS['ESPCONFIG']['ME'] ."?where=access&sid=$sid&op=p\">". mb_('Make Public') .'</a>';
		} else {
			$public = mb_('Public');
			$op     = '<a href="'. $GLOBALS['ESPCONFIG']['ME'] ."?where=access&sid=$sid&op=v\">". mb_('Make Private') .'</a>';
		}
?>
	<tr bgcolor="<?php echo($bg); ?>">
		<td><?php echo($sid); ?></td>
		<td><a href="<?php echo($GLOBALS['ESPCONFIG']['ME'] ."?where=access&sid=$sid"); ?>"><?php echo($name); ?></a></td>
		<td><?php echo($title); ?></td>
		<td><?php echo($owner); ?></td>
		<td><?php echo($realm); ?></td>
		<td><?php echo($public); ?></td>
		<td><?php echo($op); ?></td>
	</tr>
<?php
	}
?>
</table>
<?php echo("<a href=\"". $GLOBALS['ESPCONFIG']['ME'] ."?where=manage\">" . mb_('Go back to Management Interface') . "</a>\n"); ?>