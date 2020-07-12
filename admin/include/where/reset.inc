<?php

# $Id: reset.inc,v 1.3 2003/01/30 20:14:53 jimmerman Exp $

// Written by James Flemer
// For eGrad2000.com
// <jflemer@alum.rpi.edu>

	@session_unregister('survey_id');
	@session_destroy();
	include(esp_where('index'));
?>
