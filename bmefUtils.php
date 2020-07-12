<?php
// $Id: bmefUtils.php,v 1.35.1 2004/06/03 07:11:00 dashboard Exp $
if(
	!defined('XOOPS_ROOT_PATH') ||
	!defined('XOOPS_CACHE_PATH')
){
	exit();
}
include_once XOOPS_ROOT_PATH.'/modules/bmef/conf.php';

class bmefUtils {
	
	function log($str){
		if( /* defined(POPNUPBLOG_DEBUG_OUT)  && */ (POPNUPBLOG_DEBUG_OUT == 1) ){
			$log = './log/popnupsurvey.log';
			$fp = fopen($log, 'a');
			fwrite($fp, $str."\n");
			fclose($fp);
		}
	}
	
	function getDateFromHttpParams(){
		global $HTTP_POST_VARS, $HTTP_GET_VARS;
		
		$param = isset($HTTP_POST_VARS['param']) ? ($HTTP_POST_VARS['param']) : 0;
		if($param == 0){
			$param = isset($HTTP_GET_VARS['param']) ? ($HTTP_GET_VARS['param']) : 0;
		}
		/* It doesn't work with php4isapi.dll.
		if($param == 0){
			$tmp = explode('/', $_SERVER['REQUEST_URI']);
			$param = ($tmp[count($tmp)-1]);
		}*/
		$param = trim($param);
		
		if($param == 0){
			return false;
		}
		$result = array();
		$result['params'] = $param;
		//print("$param");
		if(preg_match("/^([0-9]+)-([0-9]{4})([0-9]{2})([0-9]{2})([0-9]{2})([0-9]{2})([0-9]{2})-([a-zA-Z0-9]*)/", $param, $m)){
			$result['uid'] = bmefUtils::checkUid($m[1]);
			$result['year'] = bmefUtils::checkYear($m[2]);
			$result['month'] = bmefUtils::checkMonth($m[3]);
			$result['date'] = bmefUtils::checkDate($m[2], $m[3], $m[4]);
			$result['hours']=$m[5];
			$result['minutes']=$m[6];
			$result['seconds']=$m[7];
			$result['command'] = trim($m[8]);		// enc type for MT user
		}else if(preg_match("/^([0-9]+)-([0-9]{4})([0-9]{2})([0-9]{2})([0-9]{2})([0-9]{2})([0-9]{2})/", $param, $m)){
			$result['uid'] = bmefUtils::checkUid($m[1]);
			$result['year'] = bmefUtils::checkYear($m[2]);
			$result['month'] = bmefUtils::checkMonth($m[3]);
			$result['date'] = bmefUtils::checkDate($m[2], $m[3], $m[4]);
			//print("$m[5]:$m[6]:$m[7]");
			$result['hours']=$m[5];
			$result['minutes']=$m[6];
			$result['seconds']=$m[7];
		}else if(preg_match("/^([0-9]+)-([0-9]{4})([0-9]{2})/", $param, $m)){
			$result['uid'] = bmefUtils::checkUid($m[1]);
			$result['year'] = bmefUtils::checkYear($m[2]);
			$result['month'] = bmefUtils::checkMonth($m[3]);
		}else if(preg_match("/^([0-9]+)/", $param, $m)){
			$result['uid'] = bmefUtils::checkUid($m[1]);
		}else{
			redirect_header(XOOPS_URL.'/',1,_MD_POPNUPBLOG_INVALID_DATE.'(INVALID PARAM)');
			exit();
		}
		return $result;
	}
	
	function getApplicationNum(){
		global $xoopsDB;
		if(!$dbResult = $xoopsDB->query('select count(*) num from '.POPNUPBLOG_TABLE_APPL)){
			return 0;
		}
		if(list($num) = $xoopsDB->fetchRow($dbResult)){
			return $num;
		}
		return 0;
	}
	
	function wesurveyUpdatesPing($rss, $url, $survey_name = null, $title = null, $excerpt = null){
		$ping = new bmefPing2($rss, $url, $survey_name, $title, $excerpt);
		$ping->send();
		/* debug log
		ob_start();
		print_r($ping);
		$log = ob_get_contents();
		ob_end_clean();
		bmefUtils::log($log);
		*/
	}
	
	function newApplication($in_title, $in_permission){
		global $xoopsUser, $xoopsDB;
		$title = "";
		$permission = -1;
		if(!empty($in_title)){
			$title = bmefUtils::convert2sqlString($in_title);
		}
		if( ($in_permission == 0) || ($in_permission == 1) || ($in_permission == 2) || ($in_permission == 3)){
			$permission = intval($in_permission);
		}
		
		if($permission < 0){
			return _MD_POPNUPBLOG_ERR_INVALID_PERMISSION;
		}
		if(!$result = $xoopsDB->query('select uid from '.POPNUPBLOG_TABLE_APPL.' where uid = '.$xoopsUser->uid())){
			return "select error";
		}
		if(list($tmpUid) = $xoopsDB->fetchRow($result)){
			return _MD_POPNUPBLOG_ERR_APPLICATION_ALREADY_APPLIED;
		}
		if(!$result = $xoopsDB->query('select uid from '.POPNUPBLOG_TABLE_INFO.' where uid = '.$xoopsUser->uid())){
			return "select error";
		}
		if(list($tmpUid) = $xoopsDB->fetchRow($result)){
			return _MD_POPNUPBLOG_ERR_ALREADY_WRITABLE;
		}
		$sql = sprintf("insert into %s (uid, title, permission, create_date) values(%u, '%s', %u, CURRENT_TIMESTAMP())", 
			POPNUPBLOG_TABLE_APPL, $xoopsUser->uid(), $title, $permission);
		if(!$result = $xoopsDB->query($sql)){
			return "insert error";
		}
		
		return "";
	}
	

	
	function get_survey_list($start = 0){
		global $xoopsUser, $xoopsDB;
		
		$block_list_num = 10;
		$dateFormat = '%m/%d %k:%i';
	
		$selectMax = $start + 10;
		$sql_select = sprintf('select id,name,title,owner,UNIX_TIMESTAMP(changed),subtitle FROM %s WHERE status = %u ORDER BY changed desc limit %u',
			 TABLE_SURVEY, 1, $selectMax);
		if(!$result_select = $xoopsDB->query($sql_select)){
			return false;
		}
		$tmp = array();
		$i = 0;
		$userHander = new XoopsUserHandler($xoopsDB);
		while(list($sid,$name,$title,$owner,$changed,$subtitle) = $xoopsDB->fetchRow($result_select)){
			if($i >= $start){
				$res = array();
				$res['sid'] = $sid;
				$res['name'] = $name;
				$res['title'] = $title;
				$tUser = $userHander->get($owner);
				$res['uname'] = $tUser->uname();
				$res['last_update'] = $changed;
				$res['last_update_s'] = formatTimestamp($changed, 's');
				$res['last_update_m'] = formatTimestamp($changed, 'm');
				$res['last_update_l'] = formatTimestamp($changed, 'l');
				$res['subtitle'] = $subtitle;
				$res['url'] = XOOPS_URL."/modules/bmef/public/survey.php?name=".$name;
				$tmp[$i] = $res;
			}
			$i++;
		}
		//var_dump($tmp);
		return $tmp;
	}
	
	function createRssURL($uid){
		if((empty($useRerite)) || ($useRerite == 0) ){
			return POPNUPBLOG_DIR.'rss.php'.POPNUPBLOG_REQUEST_URI_SEP.$uid;
		}else{
			return POPNUPBLOG_DIR.'rss/'.$uid.".xml";
		}
	}
	
	function createUrl($uid){
		return XOOPS_URL."/modules/bmef/";
	}
	
	function createUrlNoPath($uid, $year = 0, $month = 0, $date = 0, $hours = 0, $minutes = 0, $seconds = 0, $command = null){
		$result = '';
		if((empty($useRerite)) || ($useRerite == 0) ){
			$result .= "index.php".POPNUPBLOG_REQUEST_URI_SEP.bmefUtils::makeParams($uid, $year, $month, $date, $hours, $minutes, $seconds, $command);
		}else{
			$result .= "view/".bmefUtils::makeParams($uid, $year, $month, $date, $hours, $minutes, $seconds, $command).".html";
		}
		return $result;
	}
	
	function mb_strcut($text, $start, $end){
		if(function_exists('mb_strcut')){
			// return mb_strcut($text, $start, $end);
			return mb_substr($text, $start, $end);
		}else{
			return substr($text, $start, $end);
			// return strcut($text, $start, $end);
		}
	}
	
	function toRssDate($time, $timezone = null){
		if(!empty($timezone)){
			$time = xoops_getUserTimestamp($time, $timezone);
		}
		$res =  date("Y-m-d\\TH:i:sO", $time);
		// mmmm
		$result = substr($res, 0, strlen($res) -2).":".substr($res, -2);
		return $result;
	}
	
	function checkUid($iuid){
		$uid = intval($iuid);
		if( $uid > 0){
			return $uid;
		}
	}

	function checkYear($iyear){
		$year = intval($iyear);
		if ( ($year > 1000) && ($year < 3000) ){
			return $iyear;
		}
		redirect_header(XOOPS_URL.'/',1,_MD_POPNUPBLOG_INVALID_DATE.'(YEAR)'.$iyear);
		exit();
	}
	
	function checkMonth($imonth){
		$month = intval($imonth);
		if ( ($month > 0) && ($month < 13) ){
			return $imonth;
		}
		redirect_header(XOOPS_URL.'/',1,_MD_POPNUPBLOG_INVALID_DATE.'(MONTH)');
		exit();
	}
	
	function checkDate($year, $month, $date){
		if(checkdate(intval($month), intval($date), intval($year))){
			return $date;
		}
		redirect_header(XOOPS_URL.'/',1,_MD_POPNUPBLOG_INVALID_DATE.'(ALL DATE) '.intval($year)."-".intval($month)."-". intval($date));
		exit();
	}
	
	function makeParams($uid, $year=0, $month=0, $date=0, $hours=0, $minutes=0, $seconds=0, $command = null){
		$result = '';
		$c = '';
		if(!empty($command)){
			$c = '-'.$command;
		}
		if($year == 0){
			$result = $uid;
		}else if($date == 0){
			$result = sprintf("%s-%04u%02u%s", "".$uid, $year, $month, $c);
		}else{
			$result = sprintf("%s-%04u%02u%02u%02u%02u%02u%s", "".$uid, $year, $month, $date, $hours, $minutes, $seconds, $c);
		}
		return $result;
	}
	
	function makeTrackBackURL($uid, $year = 0, $month = 0, $date = 0, $hours=0, $minutes=0, $seconds=0){
		return XOOPS_URL.'/modules/popnupsurvey/trackback.php'.POPNUPBLOG_REQUEST_URI_SEP.bmefUtils::makeParams($uid, $year, $month, $date, $hours, $minutes, $seconds);
	}
	
	function isCompleteDate($d){
		if(!empty($d['year'])){
			if(checkdate(intval($d['month']), intval($d['date']), intval($d['year']))){
				return true;
			}
		}
		return false;
	}
	function complementDate($d){
		if(!checkdate(intval($d['month']), intval($d['date']), intval($d['year']))){
			$time = time();
			$d['year'] = date('Y',$time);
			$d['month'] = sprintf('%02u', date('m',$time));
			$d['date'] =  sprintf('%02u', date('d',$time));
			$d['hours'] =  sprintf('%02u', date('H',$time));
			$d['minutes'] =  sprintf('%02u', date('i',$time));
			$d['seconds'] =  sprintf('%02u', date('s',$time));
		}
		//print($d['hours'].$d['minutes'].$d['seconds']);
		return $d;
	}
	
	function convert_encoding(&$text, $from = 'auto', $to){
		if(function_exists('mb_convert_encoding')){
			return mb_convert_encoding($text, $to, $from); 
		} else if(function_exists('iconv')){
			return iconv($from, $to, $text);
		}else{
			return $text;
		}
	}
	
	function assign_message(&$tpl){
		$all_constants_ = get_defined_constants();
		foreach($all_constants_ as $key => $val){
			if(preg_match("/^_(MB|MD|AM|MI)_BMEF_(.)*$/", $key) || preg_match("/^BMEF_(.)*$/", $key)){
				if(is_array($tpl)){
					$tpl[$key] = $val;
				}else if(is_object($tpl)){
					$tpl->assign($key, $val);
				}
			}
		}
	}
	/*
	function get_recent_trackback($date){
		global $xoopsDB;
		$sql = 'select title, url from '.POPNUPBLOG_TABLE_TRACKBACK.' where uid = '.$date['uid'].' order by t_date desc';
		if(!$db_result = $this->xoopsDB->query($sql)){
			return false;
		}
		$i = 0;
		
		$result['html'] = '<div>';
		while(list($title, $url) = $this->xoopsDB->fetchRow($db_result)){
			$result[data][] = new array(){ 'title' => $title, 'url' => $url};
			$i++;
			$result['html'] .= '<a href="'.$url.'" target="_blank">'.$title.'</a><br />';
		}
		$result['html'] .= '</div>';
		
		return $result;
	}
	*/
	function send_trackback_ping($trackback_url, $url, $title, $survey_name, $excerpt = null) {
		bmefPing2::send_trackback_ping($trackback_url, $url, $title, $survey_name, $excerpt) ;
	}
	
	
	function remove_html_tags($t){
		return preg_replace_callback(
			"/(<[a-zA-Z0-9\"\'\=\s\/\-\~\_;\:\.\n\r\t\?\&\+\%\&]*?>|\n|\r)/ms", 
			/* "/(<[*]*?>|\n|\r)/ms", */
			"popnupsurvey_remove_html_tags_callback", 
			$t);
	}
	
	
	function convert2sqlString($text){
		$ts =& MyTextSanitizer::getInstance();
		if(!is_object($ts)){
			exit();
		}
		$res = $ts->stripSlashesGPC($text);
		$res = $ts->censorString($res);
		$res = addslashes($res);
		return $res;
	}
	function mail_popimg(){
		global $log,$limit_min;
		if (filemtime($log) < time() - $limit_min * 60) {
			return "<div style=\"text-align:center;\"><img src=./pop.php?img=1&time=".time()."\" width=70 height=1 /></div>POPed";
		} else {
			return "snoozed";
		}
	}
}
function popnupsurvey_remove_html_tags_callback($t){
	return "";
}
?>