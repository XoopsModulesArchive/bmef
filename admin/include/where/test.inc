<?php

# $Id: test.inc,v 1.15 2003/03/04 21:42:27 jimmerman Exp $

// Written by James Flemer
// For eGrad2000.com
// <jflemer@alum.rpi.edu>

	/* get the sid ... if any */
	if(!empty($HTTP_GET_VARS['sid']))
	    $sid = intval($HTTP_GET_VARS['sid']);

	if(!empty($HTTP_POST_VARS['sid']))
		$sid = intval($HTTP_POST_VARS['sid']);

    $bg = '';

	/* sid selected for testing */
	if(!empty($sid)) {
		$sql = "SELECT status,owner,realm,theme FROM ".TABLE_SURVEY." WHERE id=${sid}";
		$result = mysql_query($sql);
		list($status,$owner,$realm,$theme) = mysql_fetch_row($result);
		mysql_free_result($result);

		/* check ACL to see if user is allowed to test
		 * _this_ survey */
		$realms = array_intersect(
				$HTTP_SESSION_VARS['acl']['pall'],
				$HTTP_SESSION_VARS['acl']['pdesign']);
		if($HTTP_SESSION_VARS['acl']['superuser'] != 'Y' &&
				$owner != $xoopsUser->uid() &&
				!in_array($realm, $realms) &&
				!auth_no_access(_('to access this survey'))) {
			return;
		}
		if (($status & STATUS_TEST) &&
			!($status & (STATUS_DONE | STATUS_DELETED | STATUS_ACTIVE))) {
			/* cleared for testing ... was that enough tests? */
?>
</form>
<?php

			/* make the css style available during the testing.
			 * this should really go into the head section ...
			 * but should still work */
			if (!empty($theme))
				echo('<link rel="stylesheet" href="' .
						$GLOBALS['ESPCONFIG']['css_url'].$theme . '" type="text/css">');
?>
<b><?php mb_echo('Testing Survey...'); ?></b>
<?php echo mb_('SID')." = $sid"; ?>
<p>
<table bgcolor="#ffffff" border="1" width="95%"><tr><td>
<?php
			/* set things up for the handler to take over ... */
			$test = true;
			unset($HTTP_GET_VARS['sid']);
			unset($HTTP_POST_VARS['sid']);
			$HTTP_POST_VARS['userid'] = 'test';
			define('ESP-HANDLER-PREFIX', true);
			define('ESP-AUTH-OK', true);
			$HTTP_POST_VARS['sec'] = empty($HTTP_POST_VARS['sec']) ? 1 : intval($HTTP_POST_VARS['sec']);
			$HTTP_POST_VARS['rid'] = empty($HTTP_POST_VARS['rid']) ? 0 : intval($HTTP_POST_VARS['rid']);
			include($ESPCONFIG['handler']);
			//undefine('ESP-HANDLER-PREFIX');
			//undefine('ESP-AUTH-OK');
?>
</td></tr></table>
<a href="<?php echo($ESPCONFIG['ME']); ?>?where=results&sid=<?php echo($sid); ?>&test=1"><?php mb_echo('View Results'); ?></a><br>
<?php echo("<a href=\"". $GLOBALS['ESPCONFIG']['ME'] ."?where=manage\">" . mb_('Go back to Management Interface') . "</a>\n"); ?>
<form>
<?php
			mysql_free_result($result);
			return;
		}
	}

	/* no survey selected ... show a menu to choose from */

	/* check with ACL for allowed surveys */
	/* load names and titles of all surveys available to
	 * _this_ user */
	$statusbad = (STATUS_DONE | STATUS_DELETED | STATUS_ACTIVE);
	$statusok = STATUS_TEST;
	if($xoopsUser->isAdmin()) {
		$sql = "SELECT id,name,title,owner,realm FROM ".TABLE_SURVEY." WHERE (status & $statusok) AND NOT (status & $statusbad)
			ORDER BY id DESC";
	} else {
		$realms = array_to_insql(
			array_intersect(
				$HTTP_SESSION_VARS['acl']['pall'],
				$HTTP_SESSION_VARS['acl']['pdesign']));
		$sql = "SELECT id,name,title,owner,realm FROM ".TABLE_SURVEY." WHERE (status & $statusok) AND NOT (status & $statusbad) AND
			(owner = '". $xoopsUser->uid() ."' || realm $realms)
			ORDER BY id DESC";
	}
	$result = mysql_query($sql);

?>
<h2><?php mb_echo('Test a Survey'); ?></h2>
<?php mb_echo('Pick Survey to Test'); ?>
<table border="0" cellspacing="0" cellpadding="4" align="center" bgcolor="<?php echo($ESPCONFIG['active_bgcolor']); ?>" width="95%">
	<tr bgcolor="#dddddd">
		<th align="left"><?php mb_echo('ID'); ?></th>
		<th align="left"><?php mb_echo('Name'); ?></th>
		<th align="left"><?php mb_echo('Title'); ?></th>
		<th align="left"><?php mb_echo('Owner'); ?></th>
		<th align="left"><?php mb_echo('Group'); ?></th>
		<td>&nbsp;</td>
		<td>&nbsp;</td>
		<th align="left"><?php mb_echo('Export'); ?></th>
	</tr>
<?php
	while(list($sid,$name,$title,$owner,$realm) = mysql_fetch_row($result)) {

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
		<td><a href="<?php echo("". $GLOBALS['ESPCONFIG']['ME'] ."?where=test&sid=${sid}"); ?>"><?php mb_echo('Test'); ?></a></td>
		<td><a href="<?php echo("". $GLOBALS['ESPCONFIG']['ME'] ."?where=results&test=1&sid=${sid}"); ?>"><?php mb_echo('Results'); ?></a></td>
		<td><a href="<?php echo("". $GLOBALS['ESPCONFIG']['ME'] ."?where=export&type=csv&test=1&sid=${sid}"); ?>"><?php mb_echo('CSV'); ?></a>
		    (<a href="<?php echo("". $GLOBALS['ESPCONFIG']['ME'] ."?where=download&type=csv&test=1&sid=${sid}"); ?>"><?php mb_echo('download'); ?></a>)
			<!-- / <a href="<?php echo("". $GLOBALS['ESPCONFIG']['ME'] ."?where=export&type=dbf&test=1&sid=${sid}"); ?>"><?php mb_echo('DBF'); ?></a> -->
		</td>
	</tr>
<?php
	}
?>
</table>
<?php echo("<a href=\"". $GLOBALS['ESPCONFIG']['ME'] ."?where=manage\">" . mb_('Go back to Management Interface') . "</a>\n"); ?>