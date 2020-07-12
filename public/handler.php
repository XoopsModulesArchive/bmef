<?php
# $Id: handler.php,v 1.24.2.1 2003/04/23 00:05:22 jimmerman Exp $
// Written by James Flemer
// For eGrad2000.com
// <jflemer@alum.rpi.edu>
/* When using the authentication for responses you need to include
 * part of the script *before* your template so that the
 * HTTP Auth headers can be sent when needed.
 *
 * See the handler-prefix.php file for details.
 */
	if (!defined('ESP_BASE')) define('ESP_BASE', dirname(dirname(__FILE__)) .'/');
	require_once(ESP_BASE . '/admin/phpESP.ini.php');
	require_once($ESPCONFIG['include_path']."/funcs".$ESPCONFIG['extension']);
	require_once($ESPCONFIG['handler_prefix']);
	if(!defined('ESP-AUTH-OK')) {
		if (!empty($GLOBALS['errmsg'])) echo("handler.php[17]".$GLOBALS['errmsg']);
		return;
	}
    //esp_init_db();
	if (empty($HTTP_POST_VARS['referer'])) $HTTP_POST_VARS['referer'] = '';
	// show results instead of show survey
	// but do not allow getting results from URL or FORM
	if(isset($results) && $results) {
		// small security issue here, anyone could pick a QID to crossanalyze
		survey_results($sid,$precision,$totals,$qid,$cids);
		return;
	}
	// else draw the survey
	$sql = "SELECT status, name FROM ".TABLE_SURVEY." WHERE id='${sid}'";
	$result = mysql_query($sql);
    if ($result && mysql_num_rows($result) > 0)
    	list ($status, $name) = mysql_fetch_row($result);
    else
        $status = 0;

	if($status & ( STATUS_DONE | STATUS_DELETED )) {
		echo("handler.php[38]".mkerror(_('Error processing survey: Survey is not active.')));
		return;
	}
	if(!($status & STATUS_ACTIVE)) {
		if(!(isset($test) && $test && ($status & STATUS_TEST))) {
			echo("handler.php[43]".mkerror(_('Error processing survey: Survey is not active.')));
			return;
		}
	}

    if ($HTTP_POST_VARS['referer'] == $ESPCONFIG['autopub_url'])
        $HTTP_POST_VARS['referer'] .= "?name=$name";

	$num_sections = survey_num_sections($sid);

	$msg = '';

	$action = 'http://' . $HTTP_SERVER_VARS['HTTP_HOST'] . $HTTP_SERVER_VARS['PHP_SELF'];
	if (!empty($HTTP_SERVER_VARS['QUERY_STRING']))
		$action .= "?" . $HTTP_SERVER_VARS['QUERY_STRING'];

	if(!empty($HTTP_POST_VARS['submit'])) {
		$msg = response_check_required($sid,$HTTP_POST_VARS['sec']);
		if(empty($msg)) {
            if ($ESPCONFIG['auth_response'] && auth_get_option('resume'))
                response_delete($sid, $HTTP_POST_VARS['rid'], $HTTP_POST_VARS['sec']);
			$HTTP_POST_VARS['rid'] = response_insert($sid,$HTTP_POST_VARS['sec'],$HTTP_POST_VARS['rid']);
			response_commit($HTTP_POST_VARS['rid']);
			response_send_email($sid,$HTTP_POST_VARS['rid']);
//			goto_thankyou($sid,$HTTP_POST_VARS['referer']);
			redirect_header($HTTP_POST_VARS['referer'],2,_MD_BMEF_THANKS_ENTRY);	//$referer
			return;
		}
	}

	if(!empty($HTTP_POST_VARS['resume']) && $ESPCONFIG['auth_response'] && auth_get_option('resume')) {
        response_delete($sid, $HTTP_POST_VARS['rid'], $HTTP_POST_VARS['sec']);
		$HTTP_POST_VARS['rid'] = response_insert($sid,$HTTP_POST_VARS['sec'],$HTTP_POST_VARS['rid']);
        if ($action == $ESPCONFIG['autopub_url'])
    		goto_saved("$action?name=$name");
        else
            goto_saved($action);
		return;
	}

	if(!empty($HTTP_POST_VARS['next'])) {
		$msg = response_check_required($sid,$HTTP_POST_VARS['sec']);
		if(empty($msg)) {
            if ($ESPCONFIG['auth_response'] && auth_get_option('resume'))
                response_delete($sid, $HTTP_POST_VARS['rid'], $HTTP_POST_VARS['sec']);
			$HTTP_POST_VARS['rid'] = response_insert($sid,$HTTP_POST_VARS['sec'],$HTTP_POST_VARS['rid']);
			$HTTP_POST_VARS['sec']++;
		}
	}
	
	if (!empty($HTTP_POST_VARS['prev']) && $ESPCONFIG['auth_response'] && auth_get_option('navigate')) {
		if(empty($msg)) {
            if (auth_get_option('resume'))
                response_delete($sid, $HTTP_POST_VARS['rid'], $HTTP_POST_VARS['sec']);
			$HTTP_POST_VARS['rid'] = response_insert($sid,$HTTP_POST_VARS['sec'],$HTTP_POST_VARS['rid']);
			$HTTP_POST_VARS['sec']--;
		}
	}
    
    if ($ESPCONFIG['auth_response'] && auth_get_option('resume'))
        response_import_sec($sid, $HTTP_POST_VARS['rid'], $HTTP_POST_VARS['sec']);

	$xoopsTpl->assign('formheader', array(
		'action' => $action,
		'referer' => htmlspecialchars($HTTP_POST_VARS['referer']),
		'userid' => $HTTP_POST_VARS['userid'],
		'sid' => $sid,
		'rid' => $HTTP_POST_VARS['rid'],
		'sec' => $HTTP_POST_VARS['sec'])
	);
	// Call Render with Smarty
	$xoopsTpl->assign('formbody',survey_render_smarty($sid,$HTTP_POST_VARS['sec'],$msg));

	if ($ESPCONFIG['auth_response']) {
		if (auth_get_option('navigate') && $HTTP_POST_VARS['sec'] > 1) { 
			$xoopsTpl->assign('auth_response','<input type="submit" name="prev" value="Previous Page">');
		}
		if (auth_get_option('resume')) {
			$xoopsTpl->assign('auth_response','<input type="submit" name="resume" value="Save">');
		}
	}
	if($HTTP_POST_VARS['sec'] == $num_sections)	{
		$xoopsTpl->assign('formfooter', array('name' =>'submit','value'=>mb_('Submit Survey')));
 	} else {
		$xoopsTpl->assign('formfooter', array('name' =>'next','value'=>mb_('Next Page')));
	}
?>
