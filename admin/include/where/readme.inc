<?php

# $Id: readme.inc,v 1.5 2003/02/27 19:18:30 jimmerman Exp $

// Written by James Flemer
// For eGrad2000.com
// <jflemer@alum.rpi.edu>

?>
<?php echo("<a href=\"". $GLOBALS['ESPCONFIG']['ME'] ."?where=manage\">" . mb_('Go back to Management Interface') . "</a>\n"); ?>
<?php if (file_exists(ESP_BASE . '/README')) { ?>
<table bgcolor="#ffffff" border="0"><tr><td><pre>
<?php include(ESP_BASE . '/README'); ?>
</pre></td></tr></table>
<?php
	} else {
		echo('<p>' . mkwarn(_('README not found.')) . "</p>\n");
	}
?>
<?php echo("<a href=\"". $GLOBALS['ESPCONFIG']['ME'] ."?where=manage\">" . mb_('Go back to Management Interface') . "</a>\n"); ?>