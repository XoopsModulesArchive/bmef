<?php

# $Id: cancel.inc,v 1.5 2003/01/30 20:14:50 jimmerman Exp $

// Written by James Flemer
// For eGrad2000.com
// <jflemer@alum.rpi.edu>

	session_register('survey_id');
	session_register('survey_realm');
	if(ini_get('register_globals')) {
		$HTTP_SESSION_VARS['survey_id']    = &$survey_id;
		$HTTP_SESSION_VARS['survey_realm'] = &$survey_realm;
	}

	if(empty($HTTP_SESSION_VARS['survey_id'])) {
		// no survey INSERTed yet, so just go back to management interface
		$HTTP_SESSION_VARS['survey_id']    = '';
		$HTTP_SESSION_VARS['survey_realm'] = '';
		include(esp_where('index'));
		return;
	}

	$sql = "SELECT status FROM ".TABLE_SURVEY." WHERE	id='".$HTTP_SESSION_VARS['survey_id']."'";
	$result = mysql_query($sql);
	if(mysql_num_rows($result) < 1) {
		mysql_free_result($result);
		$HTTP_SESSION_VARS['survey_id']    = '';
		$HTTP_SESSION_VARS['survey_realm'] = '';
		include(esp_where('index'));
		return;
	}
	$status = mysql_result($result,0,0);
	mysql_free_result($result);
	$status |= STATUS_DELETED;
	$sql = "UPDATE ".TABLE_SURVEY." SET status='${status}' WHERE id='".$HTTP_SESSION_VARS['survey_id']."'";
	mysql_query($sql);
	$HTTP_SESSION_VARS['survey_id']    = '';
	$HTTP_SESSION_VARS['survey_realm'] = '';
	include(esp_where('index'));
	return;
?>