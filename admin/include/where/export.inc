<?php

# $Id: export.inc,v 1.8 2003/01/30 20:14:51 jimmerman Exp $

// Written by James Flemer
// <jflemer@alum.rpi.edu>

if(!empty($HTTP_POST_VARS['sid']))
	$sid = intval($HTTP_POST_VARS['sid']);
elseif(!empty($HTTP_GET_VARS['sid']))
	$sid = intval($HTTP_GET_VARS['sid']);
else
	$sid = '';
$bg = '';

?>
<h2><?php mb_echo('Export Data'); ?></h2>
<?php echo("<a href=\"". $GLOBALS['ESPCONFIG']['ME'] ."?where=manage\">" . mb_('Go back to Management Interface') . "</a>\n"); ?>
<table border="0" align="center" cellspacing="0" cellpadding="4" bgcolor="<?php echo($ESPCONFIG['active_bgcolor']); ?>" width="95%">
<?php

/* If the Survey ID is not given, then build
 * a menu of available surveys to chose from.
 */
if(!$sid) {
?>
	<tr bgcolor="#dddddd">
		<th align="left"><?php mb_echo('ID'); ?></th>
		<th align="left"><?php mb_echo('Name'); ?></th>
		<th align="left"><?php mb_echo('Title'); ?></th>
		<th align="left"><?php mb_echo('Owner'); ?></th>
		<th align="left"><?php mb_echo('Group'); ?></th>
		<th align="left"><?php mb_echo('Status'); ?></th>
		<th align="left" colspan="2"><?php mb_echo('Format'); ?></td>
	</tr>
<?php
	/* load names and titles of all surveys available to
	 * _this_ user */
	$statusok = (STATUS_ACTIVE | STATUS_DONE & ~STATUS_DELETED);
	if($xoopsUser->isAdmin()) {
		$sql = "SELECT id,name,title,status,owner,realm FROM ".TABLE_SURVEY."
		WHERE (status & $statusok) ORDER BY id DESC";
	} else {
		$realms = array_to_insql(
			array_intersect(
				$HTTP_SESSION_VARS['acl']['pall'],
				$HTTP_SESSION_VARS['acl']['pdata']));
		$sql = "SELECT id,name,title,status,owner,realm
			FROM ".TABLE_SURVEY." WHERE (status & $statusok) AND (owner = '".
			$xoopsUser->uid() ."' || realm $realms) ORDER BY id DESC";
	}
	$result = mysql_query($sql);

	while(list($sid,$name,$title,$status,$owner,$realm) = mysql_fetch_row($result)) {
		$stat = mb_('Editing');

		if($status & STATUS_DELETED) {
			$stat = mb_('Archived');
			continue;
		} elseif($status & STATUS_DONE) {
			$stat = mb_('Ended');
		} elseif($status & STATUS_ACTIVE) {
			$stat = mb_('Active');
		} elseif($status & STATUS_TEST) {
			$stat = mb_('Testing');
		}

		if($bg != $ESPCONFIG['bgalt_color1'])
			$bg = $ESPCONFIG['bgalt_color1'];
		else
			$bg = $ESPCONFIG['bgalt_color2'];
?>
	<tr bgcolor="<?php echo($bg); ?>">
		<td><?php echo($sid); ?></td>
		<td><?php echo($name); ?></td>
		<td><?php echo($title); ?></td>
		<td><?php echo($owner); ?></td>
		<td><?php echo($realm); ?></td>
		<td><?php echo($stat); ?></td>
		<td><?php
			echo("<a href=\"". $GLOBALS['ESPCONFIG']['ME'] ."?where=export&type=csv&sid=$sid\">" . mb_('CSV') . "</a>"); ?>
			(<?php
			echo("<a href=\"". $GLOBALS['ESPCONFIG']['ME'] ."?where=download&type=csv&sid=$sid\">" . mb_('download') . '</a>'); ?>)</td>
		<td>&nbsp;<!-- <?php
			echo("<a href=\"". $GLOBALS['ESPCONFIG']['ME'] ."?where=export&type=dbf&sid=$sid\">" . mb_('DBF') . "</a>"); ?>  --></td>
	</tr>
<?php
	}
?>
</table>
<?php echo("<a href=\"". $GLOBALS['ESPCONFIG']['ME'] ."?where=manage\">" . mb_('Go back to Management Interface') . "</a>\n"); ?>
<?php
	return;
	}

	/* sid supplied ... continue */
	$sql = "SELECT name FROM ".TABLE_SURVEY." WHERE id = $sid";
	$result = mysql_query($sql);
	$name = mysql_result($result, 0, 0);
	mysql_free_result($result);

	/* check ACLs for permissions */
	$srealm = auth_get_survey_realm($sid);
	if(isset($HTTP_GET_VARS['test'])) {
		/* check ACL to see if user is allowed to test
		 * _this_ survey */
		if($HTTP_SESSION_VARS['acl']['superuser'] != 'Y' &&
				!auth_is_owner($sid, $xoopsUser->uid()) &&
				!in_array($srealm, array_intersect(
						$HTTP_SESSION_VARS['acl']['pdesign'],
						$HTTP_SESSION_VARS['acl']['pall'])) &&
				!auth_no_access(_('to access this survey'))) {
			return;
		}
		echo("<p><b>". mb_('Testing Survey...') ."</b> (". mb_('SID') ." = $sid)</p>\n");
	} else {
		/* check ACL to see if user is allowed to export
		 * _this_ survey */
		if($HTTP_SESSION_VARS['acl']['superuser'] != 'Y' &&
				!auth_is_owner($sid, $xoopsUser->uid()) &&
				!in_array($srealm, array_intersect(
						$HTTP_SESSION_VARS['acl']['pdata'],
						$HTTP_SESSION_VARS['acl']['pall'])) &&
				!auth_no_access(_('to access this survey'))) {
			return;
		}
	}

	$type = isset($HTTP_GET_VARS['type']) ? $HTTP_GET_VARS['type'] : 'csv';

	// Try DBF first, default to CSV
	if($type == 'dbf') {
		$file = $HTTP_SESSION_VARS['acl']['home'] . "/$name.dbf";
		if(extension_loaded('dbase')) {
			$ret = survey_export_dbf($sid, $file);
		} else {
			$ret = 0;
			echo("<tr><td>" . mkerror(_('The PHP dBase Extension is not installed.')) . "</td></tr>");
		}
	} else {
		$file = $HTTP_SESSION_VARS['acl']['home'] . "/$name.csv";
		$ret = survey_export_csv($sid, $file);
	}

	echo("<tr><td>");
	if(isset($HTTP_GET_VARS['test'])) {
		echo("<p><b>". mb_('Testing Survey...') ."</b> (". mb_('SID') ." = $sid)</p>\n");
	}
	if($ret) {
		echo(mb_('Survey exported as:') . " <tt>$file</tt>");
	} else {
		echo(mkwarn(_('Error exporting survey as:') . " <tt>$file</tt>"));
	}
	echo("</td></tr></table>\n");
	echo("<a href=\"". $GLOBALS['ESPCONFIG']['ME'] ."?where=manage\">" . mb_('Go back to Management Interface') . "</a>\n");

?>