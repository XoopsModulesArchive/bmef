<?php

# $Id: guide.inc,v 1.3 2003/02/27 19:17:31 jimmerman Exp $

// Written by James Flemer
// For eGrad2000.com
// <jflemer@alum.rpi.edu>

?>
<?php echo("<a href=\"". $GLOBALS['ESPCONFIG']['ME'] ."?where=manage\">" . mb_('Go back to Management Interface') . "</a>\n"); ?>
<?php if (file_exists(ESP_BASE . '/docs/GUIDE')) { ?>
<table bgcolor="#ffffff" border="0"><tr><td><pre>
<?php include(ESP_BASE . '/docs/GUIDE'); ?>
</pre></td></tr></table>
<?php
	} else {
		echo('<p>' . mkwarn(_("User's guide not found.")) . "</p>\n");
	}
?>
<?php echo("<a href=\"". $GLOBALS['ESPCONFIG']['ME'] ."?where=manage\">" . mb_('Go back to Management Interface') . "</a>\n"); ?>