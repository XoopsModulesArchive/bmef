<?php

# $Id: logout.inc,v 1.2 2003/01/30 20:14:51 jimmerman Exp $

// Written by James Flemer
// <jflemer@alum.rpi.edu>
// <jflemer@acm.rpi.edu>

	manage_logout();
?>
<table border="0" cellspacing="0" cellpadding="4" align="center" bgcolor="<?php echo($ESPCONFIG['active_bgcolor']); ?>" width="95%">
	<tr>
		<td>Logged out.<br>
			<?php echo("<a href=\"". $GLOBALS['ESPCONFIG']['ME'] ."?where=manage\">" . mb_('Log back in.') . "</a>\n"); ?>
		</td>
	</tr>
</table>
&nbsp;