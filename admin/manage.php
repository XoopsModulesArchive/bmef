<?php
// # $Id: manage.php,v 1.17 2003/03/04 14:40:32 jimmerman Exp $
// Written by James Flemer For eGrad2000.com <jflemer@alum.rpi.edu>
require('../../../mainfile.php');
require(XOOPS_ROOT_PATH.'/header.php');
//include '../../../include/cp_header.php';
include '../conf.php';


	if(!$xoopsUser || !is_object($xoopsUser)){
		redirect_header(XOOPS_URL.'/modules/bmef/',2,_MD_BMEF_CAN_WRITE_USER_ONLY);
		exit();
	}

	if (!defined('ESP_BASE'))
		define('ESP_BASE', dirname(dirname(__FILE__)) .'/');

 	$CONFIG = ESP_BASE . '/admin/phpESP.ini.php';

	if(!file_exists($CONFIG)) {
		echo("<b>FATAL: Unable to open $CONFIG. Aborting.</b>");
		exit;
	}
	if(!extension_loaded('mysql')) {
		echo('<b>FATAL: Mysql extension not loaded. Aborting.</b>');
		exit;
	}
	require_once($CONFIG);
	
	//esp_init_db();
	
	session_register('acl');
	if(get_cfg_var('register_globals')) {
		$HTTP_SESSION_VARS['acl'] = &$acl;
	}
	if($ESPCONFIG['auth_design']) {
		if(!manage_auth(
				_addslashes(@$HTTP_SERVER_VARS['PHP_AUTH_USER']),
				_addslashes(@$HTTP_SERVER_VARS['PHP_AUTH_PW'])))
			exit;
	} else {
		$HTTP_SESSION_VARS['acl'] = array (
			'username'  => $xoopsUser->uid(),
			'pdesign'   => array('none'),
			'pdata'     => $member_handler->getGroupsByUser( $xoopsUser->uid() ),
			'pstatus'   => array('none'),
			'pall'      => $member_handler->getGroupsByUser( $xoopsUser->uid() ),
			'pgroup'    => array('none'),
			'puser'     => array('none'),
			'superuser' => 'Y',
			'disabled'  => 'N'
		);
		//$member_handler->getGroupsByUser( $xoopsUser->uid() )
	}
	
	$where = '';
	if(isset($HTTP_POST_VARS['where']))
		$where = $HTTP_POST_VARS['where'];
	elseif(isset($HTTP_GET_VARS['where']))
		$where = $HTTP_GET_VARS['where'];

	if ($where == 'download') {
		include(esp_where($where));
		exit;
	}

	//xoops_cp_header();

	if(!empty($ESPCONFIG['style_sheet'])) {
		echo("<link href=\"". $ESPCONFIG['style_sheet'] ."\" rel=\"stylesheet\" type=\"text/css\" />\n");
	}
	if(!empty($ESPCONFIG['charset'])) {
		echo('<meta http-equiv="Content-Type" content="text/html; charset='. $ESPCONFIG['charset'] ."\" />\n");
	}
	if($ESPCONFIG['DEBUG']) {
		include($ESPCONFIG['include_path']."/debug".$ESPCONFIG['extension']);
	}

	if(file_exists($ESPCONFIG['include_path']."/head".$ESPCONFIG['extension']))
		include($ESPCONFIG['include_path']."/head".$ESPCONFIG['extension']);
	// This is the body admin
	//echo "manage include:".esp_where($where);
	include(esp_where($where));
	//echo $HTTP_SESSION_VARS['survey_id'];

	if(file_exists($ESPCONFIG['include_path']."/foot".$ESPCONFIG['extension']))
		include($ESPCONFIG['include_path']."/foot".$ESPCONFIG['extension']);

//	xoops_cp_footer();
	include(XOOPS_ROOT_PATH.'/footer.php');
	exit;
?>