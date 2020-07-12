<?php

# $Id: passwd.inc,v 1.4 2003/01/31 04:13:04 jimmerman Exp $

// Written by James Flemer
// <jflemer@alum.rpi.edu>
// <jflemer@acm.rpi.edu>

	/* ACL: everyone is allowed to change her own password */
?>
<h2><?php mb_echo('Change Password'); ?></h2>
<?php
	if(isset($HTTP_POST_VARS['newpass1']) &&
		$HTTP_POST_VARS['newpass1'] == $HTTP_POST_VARS['newpass2'] &&
		!empty($HTTP_POST_VARS['newpass1'])) {
		if(auth_change_manager_passwd(
				$xoopsUser->uid(),
				_addslashes($HTTP_POST_VARS['oldpass']),
				_addslashes($HTTP_POST_VARS['newpass1']))) {
			mb_echo('Your password has been successfully changed.') ."<br>\n");
			echo("<a href=\"". $GLOBALS['ESPCONFIG']['ME'] ."?where=manage\">" . mb_('Go back to Management Interface') . "</a>\n");
			return;
		} else {
			echo(mkerror(_('Password not set, check your old password.'));
		}
	} else if(isset($HTTP_POST_VARS['newpass1'])) {
		echo(mkerror(_('New passwords do not match or are blank.')));
	}
?>
<input type="hidden" name="where" value="passwd">
<table border="0" cellspacing="0" cellpadding="4" align="center" bgcolor="<?php echo($ESPCONFIG['active_bgcolor']); ?>">
<tr>
	<th align="right" bgcolor="<?php echo($ESPCONFIG['bgalt_color2']); ?>">
	<?php mb_echo('Username'); ?></th>
	<td bgcolor="<?php echo($ESPCONFIG['bgalt_color1']); ?>">
	<?php echo('<tt>'. $xoopsUser->uid() .'</tt>'); ?></td>
</tr><tr>
	<th align="right" bgcolor="<?php echo($ESPCONFIG['bgalt_color2']); ?>">
	<?php mb_echo('Old Password'); ?></th>
	<td bgcolor="<?php echo($ESPCONFIG['bgalt_color1']); ?>">
	<?php echo(mkpass('oldpass')); ?></td>
</tr><tr>
	<th align="right" bgcolor="<?php echo($ESPCONFIG['bgalt_color2']); ?>">
	<?php mb_echo('New Password'); ?></th>
	<td bgcolor="<?php echo($ESPCONFIG['bgalt_color1']); ?>">
	<?php echo(mkpass('newpass1')); ?></td>
</tr><tr>
	<th align="right" bgcolor="<?php echo($ESPCONFIG['bgalt_color2']); ?>">
	<?php mb_echo('Confirm New Password'); ?></th>
	<td bgcolor="<?php echo($ESPCONFIG['bgalt_color1']); ?>">
	<?php echo(mkpass('newpass2')); ?></td>
</tr><tr>
	<th colspan="2" bgcolor="<?php echo($ESPCONFIG['bgalt_color2']); ?>">
	<input type="submit" value="<?php mb_echo('Change Password'); ?>">
	</th>
</tr>
</table>
<?php echo("<a href=\"". $GLOBALS['ESPCONFIG']['ME'] ."?where=manage\">" . mb_('Go back to Management Interface') . "</a>\n"); ?>