<?php

# $Id: espresponse.inc,v 1.28.2.1 2003/04/22 23:54:21 jimmerman Exp $
// Written by James Flemer
// <jflemer@alum.rpi.edu>

/* {{{ proto string response_check_required(int survey_id, int section)
   Reads current form variables from HTTP_POST_VARS.
   Returns an empty string if all required fields are
   completed, else returns a message string indicating which
   questions need to be completed. */

//
// Modified by Y.Sakai webmaster@bluemooninc.biz
// 2004/5/27 Add for attach
//
function get_attach($uname,$qid) {
	global $_FILES,$ESPCONFIG;
	// Upload folder
	$uploads = XOOPS_ROOT_PATH.$ESPCONFIG['attach_path'];
	// Acceptable MIME Content-Type
	$subtype = "gif|jpe?g|png|bmp|zip|lzh|pdf|excel|powerpoint|octet-stream|x-pmd|x-mld|x-mid|x-smd|x-smaf|x-mpeg";
	// embedding image MIME Content-Type
	$imgtype = "gif|jpe?g|png|bmp|x-pmd|x-mld|x-mid|x-smd|x-smaf|x-mpeg";
	// Reject Ext. name
	$viri = "cgi|php|jsp|pl|htm";
	$upfile_url = "";
	// You must set at php.ini 'register_globals = on;'
	$upfile	  = $_FILES[$qid];				//upload file object 
	$upfile_tmp  = $_FILES[$qid]['tmp_name'];	//tmp file name 
	$upfile_name = $_FILES[$qid]['name'];		//Local File Name 
	$upfile_size = $_FILES[$qid]['size'];		//File Size
	$upfile_type = $_FILES[$qid]['type'];		//File MIME Content-Type
	//Disp File Infomation for debug
	//print("Start get_attach: $uname,$qid<BR>\n");
	//print("File Infomation:<BR>\n");
	//print("File From : $upfile_tmp<BR>\n");
	//print("File To : $upfile_name<BR>\n");
	//print("File Size - $upfile_size<BR>\n");
	//print("File type - $upfile_type<BR>\n");
	if ($upfile_tmp != "" ){
		if (eregi($subtype, $upfile_type)){ $deny=0; } else { $deny=1; };
		$ext = strtolower(end(explode(".",$upfile_name))); 
		if (eregi($viri, $ext)) $deny = 1;
		if (eregi($imgtype, $upfile_type)){
			$upfile_localname= $uname."_".$qid."_".time()."_".cnv_mbstr($upfile_name);		// convert for mbstrings
			$upfile_url = "<img src='".$uploads.rawurlencode($upfile_localname)."'>".$upfile_name;	//rawurlencode
			copy($upfile_tmp,$uploads.$upfile_localname);
		} elseif ($deny==0) {
			$upfile_localname= $uname."_".${qid}."_".$upfile_name.".".time();
			$upfile_url = "<a href='".$uploads.rawurlencode($upfile_localname)."' target='_blank'>".$upfile_name."</a>";
			copy($upfile_tmp,$uploads.$upfile_localname);
		} else {
			$upfile_url = "Upload Denied...".$upfile_type." ext ".$ext;
		}
	}
	return $upfile_url;
}

function response_check_required($sid, $section) {
	global $HTTP_POST_VARS;

	$sql = "SELECT id,type_id,content FROM ".TABLE_QUESTION." ".
		survey_select_section_sql($sid,$section)." AND required='Y' AND deleted='N' ORDER BY position";
	$result = mysql_query($sql);
	if(mysql_num_rows($result) < 1) {
		// no required fields! so no need to continue
		mysql_free_result($result);
		return('');
	}

	$missing = array();	// array of missing questions

	while(list($qid,$tid,$content) = mysql_fetch_row($result)) {
		if($tid == 8) { // Rank
			$sql = "SELECT id FROM ".TABLE_QUESTION_CHOICE." WHERE question_id='$qid'";
			$cid_result = mysql_query($sql);
			while(list($cid) = mysql_fetch_row($cid_result)) {
				if(!isset($HTTP_POST_VARS["${qid}_${cid}"])) {
					$missing[$qid] = $content;
					break;
				}
			}
			mysql_free_result($cid_result);
			continue;
		}
		if ($tid == 10 && $HTTP_POST_VARS[$qid] === '0') // Numeric
			continue;
		if(empty($HTTP_POST_VARS[$qid]))
			$missing[$qid] = $content;
	}
	mysql_free_result($result);

	if(count($missing) > 0) {
		// missing required variables
		$message = mb_('You are missing the following required questions:') ."<br>\n";
		while(list($qid,$content)=each($missing)) {
			if($GLOBALS['ESPCONFIG']['DEBUG'])
				$message .= "<!-- ${qid} -->";
			$message .= "${content}<br>\n";
		}
		return($message);
	}
	return('');
}
/* }}} */

/* {{{ proto void response_delete(int survey_id, int section, int response_id)
   Deletes values for the response. */
function response_delete($sid, $rid, $sec = null) {
	if (empty($rid))
		return;

	if ($sec != null) {
		if ($sec < 1)
			return;

		/* get question_id's in this section */
		$qids = survey_get_sections($sid);
		if (!isset($qids[$sec - 1]))
			return;
		$qids = 'AND question_id '. array_to_insql($qids[$sec - 1]);
	} else {
		/* delete all */
		$qids = '';
	}

	/* delete values */
	foreach (array('bool', 'single', 'multiple', 'rank', 'text', 'other', 'date') as $tbl) {
		$sql = "DELETE FROM response_$tbl WHERE response_id = '$rid' $qids";
		$res = mysql_query($sql);
	}
}
/* }}} */

/* {{{ proto void response_delete_all(int survey_id)
   Deletes all responses from survey. */
function response_delete_all($sid) {
	$sec = survey_get_sections($sid);
	$qids = array();
	foreach ($sec as $s)
		$qids = array_merge($qids, $s);
	$qids = array_to_insql($qids);

	/* delete values */
	foreach (array('bool', 'single', 'multiple', 'rank', 'text', 'other', 'date') as $tbl) {
		$sql = "DELETE FROM response_$tbl WHERE question_id $qids";
		$res = mysql_query($sql);
	}
		/* ensure responses from testing status are also deleted */
		$sql = "DELETE FROM ".TABLE_RESPONSE." WHERE survey_id=${sid}";
		mysql_query($sql);
}
/* }}} */

/* {{{ proto int response_insert(int survey_id, int section, int response_id)
   Reads current form variables from HTTP_POST_VARS.
   Returns the ID for the response. */
function response_insert($sid,$section,$rid) {
	global $HTTP_POST_VARS,$xoopsUser,$xoopsDB;

	$userid = isset($GLOBALS['HTTP_SERVER_VARS']['PHP_AUTH_USER']) ?
			$GLOBALS['HTTP_SERVER_VARS']['PHP_AUTH_USER'] : '';
	if (empty($userid))
		$userid = $HTTP_POST_VARS['userid'];

	if(empty($rid)) {
		// create a uniqe id for this response
		$sql = "INSERT INTO ".TABLE_RESPONSE." (survey_id,username) VALUES ( '${sid}','${userid}' )";
		$result = mysql_query($sql);
		$rid = mysql_insert_id();
	}

	$sql  = "SELECT Q.id, Q.type_id, T.response_table FROM ".TABLE_QUESTION.
		" Q, ".TABLE_QUESTION_TYPE." T ". survey_select_section_sql($sid,$section,'Q') .
		" AND Q.survey_id='${sid}' AND Q.deleted='N' AND Q.type_id < 50 AND Q.type_id=T.id";
	$q_result = mysql_query($sql);
	while(list($qid, $tid, $table) = mysql_fetch_row($q_result)) {
		if (isset($HTTP_POST_VARS[$qid])) {
			if (is_array($HTTP_POST_VARS[$qid]))
				$bareval = array_map('_stripslashes', $HTTP_POST_VARS[$qid]);
			else
				$bareval = _stripslashes($HTTP_POST_VARS[$qid]);
		}
		else
			$bareval = '';

		if (is_string($bareval))
			$val = addslashes($bareval);
			else
					$val = '';
		switch($table ) {
			case 'response_bool':
				$sql  = "INSERT INTO ".TABLE_."${table} ( response_id,question_id,choice_id ) VALUES ( '${rid}','${qid}','${val}' )";
						if (!empty($val))
						$result = mysql_query($sql);
				break;
			case 'response_text':
				// only insert if non-empty content
				if($tid == 10) { // numeric
					$bareval = ereg_replace("[^0-9.\-]*(-?[0-9]*\.?[0-9]*).*", '\1', $bareval);
				}
				if($tid == 40) { // attach by Y.Sakai
					if ($xoopsUser){
						$userHander = new XoopsUserHandler($xoopsDB);
						$tUser = $userHander->get($xoopsUser->uid());
						$uname = $tUser->uname();
					}else{
						$uname = "Anonymous";
					}
					$bareval = get_attach($uname,$qid);
				}
				if(ereg("[^ \t\n]",$bareval)) {
					$val = addslashes($bareval);
					$sql = "INSERT INTO ".TABLE_."${table} ( response_id,question_id,response ) VALUES ( '${rid}','${qid}','${val}' )";
					$result = mysql_query($sql);
				}
				break;
			case 'response_date':
				// only insert if non-empty content
				if(($bareval = ereg_replace("[^0-9]*([0-9]+)/([0-9]+)/([0-9]+).*", '\1-\2-\3', $bareval))) {
					$val = addslashes($bareval);
					$sql  = "INSERT INTO ".TABLE_."${table} ( response_id,question_id,response ) VALUES ( '${rid}','${qid}','${val}' )";
					$result = mysql_query($sql);
				}
				break;
			case 'response_single':
				if(empty($bareval)) {
					$sql = "SELECT id FROM ".TABLE_QUESTION_CHOICE." WHERE question_id='${qid}' AND content LIKE '!other%' ORDER BY id";
					$c_result = mysql_query($sql);
					while(list($cid) = mysql_fetch_row($c_result)) {
									if (!isset($HTTP_POST_VARS["${qid}_${cid}"]))
											continue;
						$other = _addslashes($HTTP_POST_VARS["${qid}_${cid}"]);
						if(ereg("[^ \t\n]",$other)) {
							$sql = "INSERT INTO ".TABLE_RESPONSE_OTHER."  ( response_id,question_id,choice_id,response ) VALUES ( '${rid}','${qid}','${cid}','${other}' )";
							$result = mysql_query($sql);
							$val = $cid;
							break;
						}
					}
					mysql_free_result($c_result);
				}
				if(ereg("other_([0-9]+)", $bareval, $regs)) {
					$cid=$regs[1];
								if (!isset($HTTP_POST_VARS["${qid}_${cid}"]))
									break; // out of the case
					$other = _addslashes($HTTP_POST_VARS["${qid}_${cid}"]);
					if(ereg("[^ \t\n]",$other)) {
						$sql = "INSERT INTO ".TABLE_RESPONSE_OTHER."  ( response_id,question_id,choice_id,response ) VALUES ( '${rid}','${qid}','${cid}','${other}' )";
						$result = mysql_query($sql);
						$val=$cid;
					}
				}
				$sql  = "INSERT INTO ".TABLE_."${table} ( response_id,question_id,choice_id ) VALUES ( '${rid}','${qid}','${val}' )";
				$result = mysql_query($sql);
				break;
			case 'response_multiple':
				$sql = "SELECT id FROM ".TABLE_QUESTION_CHOICE." WHERE question_id='${qid}' AND content LIKE '!other%'";
				$c_result = mysql_query($sql);
				while(list($cid) = mysql_fetch_row($c_result)) {
							if (!isset($HTTP_POST_VARS["${qid}_${cid}"]) || empty($HTTP_POST_VARS["${qid}_${cid}"]))
								continue;
							if (!isset($HTTP_POST_VARS[$qid]))
								$HTTP_POST_VARS[$qid] = array($cid);
							else
								array_push($HTTP_POST_VARS[$qid], $cid);
					$other = _addslashes($HTTP_POST_VARS["${qid}_${cid}"]);
					if(ereg("[^ \t\n]",$other)) {
						$sql = "INSERT INTO ".TABLE_RESPONSE_OTHER."  ( response_id,question_id,choice_id,response ) VALUES ( '${rid}','${qid}','${cid}','${other}' )";
						$result = mysql_query($sql);
					}
				}
				mysql_free_result($c_result);
				if(!isset($HTTP_POST_VARS[$qid]) || count($HTTP_POST_VARS[$qid]) < 1)
					break;
				foreach($HTTP_POST_VARS[$qid] as $cid) {
					$cid = _addslashes($cid);
					if(ereg("other_[0-9]+", $cid))
						continue;
					$sql  = "INSERT INTO ".TABLE_."${table} ( response_id,question_id,choice_id ) VALUES ( '${rid}','${qid}','${cid}' )";
					$result = mysql_query($sql);
				}
				break;
			case 'response_rank':
				if($tid == 8) { // Rank
					$sql = "SELECT id FROM ".TABLE_QUESTION_CHOICE." WHERE question_id='${qid}'";
					$cid_result = mysql_query($sql);
					while(list($cid) = mysql_fetch_row($cid_result)) {
									if (!isset($HTTP_POST_VARS["${qid}_${cid}"]))
											continue;
						$val = _addslashes($HTTP_POST_VARS["${qid}_${cid}"]);
						if(strtolower($val) == "n/a")
							$rank = -1;
						else
							$rank = intval($val);
						$sql  = "INSERT INTO ".TABLE_."${table} ( response_id,question_id,choice_id,rank ) ";
						$sql .= "VALUES ( '${rid}','${qid}','${cid}','${rank}' )";
						mysql_query($sql);
					}
					mysql_free_result($cid_result);
					break;
				}
				if(strtolower($bareval) == "n/a")
					$rank = -1;
				else
					$rank = intval($bareval);
				$sql  = "INSERT INTO ".TABLE_."${table} ( response_id,question_id,rank ) VALUES ( '${rid}','${qid}','${rank}' )";
				$result = mysql_query($sql);
				break;
		}
	}
	mysql_free_result($q_result);
	return($rid);
}
/* }}} */

/* {{{ proto bool response_commit(int response_id)
   Marks a response as completed.  Returns true on success.
 */
function response_commit($rid) {
	$sql = "UPDATE ".TABLE_RESPONSE." SET complete='Y', submitted = NULL WHERE id='${rid}'";
	if(mysql_query($sql)) return(true);
	return(false);
}
/* }}} */

/* {{{ proto int response_key_cmp(mixed left, mixed right)
   Key comparator for response keys (to handle numeric, and strings
   composed of QID_CID).
 */
function response_key_cmp($l, $r) {
	$lx = explode('_', $l);
	$rx = explode('_', $r);
	$lc = intval($lx[0]);
	$rc = intval($rx[0]);
	if ($lc == $rc) {
		if (count($lx) > 1 && count($rx) > 1) {
			$lc = intval($lx[1]);
			$rc = intval($rx[1]);
		} else if (count($lx) > 1) {
			$lc++;
		} else if (count($rx) > 1) {
			$rc++;
		}
	}
	if ($lc == $rc)
		return 0;
	return ($lc > $rc) ? 1 : -1;
}
/* }}} */

/* {{{ proto array response_select(int survey_id, int response_id, array columns, array question_ids)
   Returns the values from the specific response in a sorted
   (associative) array indexed by question_id.
   
   The key in the associative array is the Question ID, or in the case
   of "!other", or rank questions the Question ID concatenated with the
   Choice ID ("${qid}_${cid").  The value for all types is an array. 
   This array contains any columns requested by the parameter $col as
   the first elements.  The last two elements are the response content,
   and the content ID respectively.  (For multiple answer questions,
   the value is an array of these arrays.)
   
   For example, for a boolean type question (ID #42) and a check box
   type (ID #44), you might get something like:

	  $response_select = array(
		'42' => array(_('No'), 'N'),
		'44' => array(
				  array('Blue', 102),
				  array('Green', 104)
				)
		);

   When in doubt, consult var_dump() as to the format of the returned
   array.
 */
function response_select($sid, $rid, $col = null, $qids = null,$order = null) {
	$values = array();

	if ($col == null) {
		$col = '';
	}
	if (!is_array($col) && !empty($col)) {
		$col = explode(',', preg_replace("/\s/",'', $col));
	}
	if (is_array($col) && count($col) > 0) {
		$col = ',' . implode(',', array_map(create_function('$a','return "q.$a";'), $col));
	}

	if ($qids == null) {
		$qids = '';
	} elseif (is_array($qids)) {
		$qids = 'AND a.question_id ' . array_to_insql($qids);
	} elseif (intval($qids) > 0) {
		$qids = 'AND a.question_id = ' . intval($qids);
	} else {
		$qids = '';
	}
	if ($order == null) {
		$order = 'q.id';
	}
	// --------------------- response_bool ---------------------
	$sql = "SELECT $order $col,a.choice_id FROM ".TABLE_RESPONSE_BOOL." a, ".
		TABLE_QUESTION." q WHERE a.response_id='${rid}' AND a.question_id=q.id $qids";
	$result = mysql_query($sql);
	while($row = mysql_fetch_row($result)) {
		$qid = array_shift($row);
		$val = array_pop($row);
		$values[$qid] = $row;
		array_push($values["$qid"], ($val == 'Y') ? mb_('Yes') : mb_('No'), $val);
	}
	mysql_free_result($result);

	// --------------------- response_single ---------------------
	$sql = "SELECT $order $col,c.content,c.id FROM ".TABLE_RESPONSE_SINGLE." a, ".
		TABLE_QUESTION." q, ".TABLE_QUESTION_CHOICE." c WHERE a.response_id='${rid}' AND a.question_id=q.id AND a.choice_id=c.id $qids";
	$result = mysql_query($sql);
	while($row = mysql_fetch_row($result)) {
		$qid = array_shift($row);
		$c = count($row);
		$val = $row[$c - 2];
		if (ereg('^!other', $val))
			$row[$c - 1] = 'other_' . $row[$c - 1];
		else
			settype($row[$c - 1], 'integer');
		$values[$qid] = $row;
	}
	mysql_free_result($result);

	// --------------------- response_multiple ---------------------
	$sql = "SELECT $order $col, c.content,c.id FROM ".
		TABLE_RESPONSE_MULTIPLE."  a, ".TABLE_QUESTION." q, ".TABLE_QUESTION_CHOICE.
		" c WHERE a.response_id='${rid}' AND a.question_id=q.id AND a.choice_id=c.id $qids ORDER BY a.question_id,c.id";
	$result = mysql_query($sql);
	$arr = array();
	$tmp = null;
	while($row = mysql_fetch_row($result)) {
		$qid = array_shift($row);
		$c = count($row);
		$val = $row[$c - 2];
		if (ereg('^!other', $val))
			$row[$c - 1] = 'other_' . $row[$c - 1];
		else
			settype($row[$c - 1], 'integer');
		if($tmp == $qid) {
			$arr[] = $row;
			continue;
		}
		if($tmp != null)
			$values["$tmp"]=$arr;
		$tmp = $qid;
		$arr = array($row);
	}
	if($tmp != null)
		$values["$tmp"]=$arr;
	mysql_free_result($result);
	unset($arr);
	unset($tmp);
	unset($row);

	// --------------------- response_other ---------------------
	$sql = "SELECT $order, c.id $col, a.response FROM ".TABLE_RESPONSE_OTHER.
		" a, ".TABLE_QUESTION." q, ".TABLE_QUESTION_CHOICE.
		" c WHERE a.response_id='${rid}' AND a.question_id=q.id AND a.choice_id=c.id $qids ORDER BY a.question_id,c.id";
	$result = mysql_query($sql);
	while($row = mysql_fetch_row($result)) {
		$qid = array_shift($row);
		$cid = array_shift($row);
		array_push($row, $row[count($row) - 1]);
		$values["${qid}_${cid}"] = $row;
	}
	mysql_free_result($result);

	// --------------------- response_rank ---------------------
	$sql = "SELECT CONCAT($order,'_',c.id) AS id $col,c.content,a.rank FROM ".
		TABLE_RESPONSE_RANK."  a, ".TABLE_QUESTION." q, ".TABLE_QUESTION_CHOICE.
		" c WHERE a.response_id='${rid}' AND a.question_id=q.id AND a.choice_id=c.id $qids ORDER BY a.question_id,c.id";
	$result = mysql_query($sql);
	while($row = mysql_fetch_row($result)) {
		$qid = array_shift($row);
		settype($row[count($row) - 1], 'integer');
		$values[$qid] = $row;
	}
	mysql_free_result($result);

	// --------------------- response_text ---------------------
	$sql = "SELECT $order $col,a.response FROM ".TABLE_RESPONSE_TEXT." a, ".
		TABLE_QUESTION." q WHERE a.response_id='${rid}' AND a.question_id=q.id $qids";
	$result = mysql_query($sql);
	while($row = mysql_fetch_row($result)) {
		$qid = array_shift($row);
		$values["$qid"]=$row;
		$val = array_pop($values["$qid"]);
		array_push($values["$qid"], $val, $val);
	}
	mysql_free_result($result);

	// --------------------- response_date ---------------------
	$sql = "SELECT $order $col,a.response FROM ".TABLE_RESPONSE_DATE." a, ".
		TABLE_QUESTION." q WHERE a.response_id='${rid}' AND a.question_id=q.id $qids";
	$result = mysql_query($sql);
	while($row = mysql_fetch_row($result)) {
		$qid = array_shift($row);
		$values["$qid"]=$row;
		$val = array_pop($values["$qid"]);
		array_push($values["$qid"], $val, $val);
	}
	mysql_free_result($result);

	// --------------------- return ---------------------
	uksort($values, 'response_key_cmp');
	return($values);
}
/* }}} */

/* {{{ proto array response_select_human(int survey_id, int response_id, array question_ids)
   A wrapper around response_select(), that returns an array of
   key/value pairs more suitable for humans.
 */
function response_select_human($sid, $rid, $qids = null) {
	$res = response_select($sid, $rid, 'type_id,content', $qids,'position');	//
	//var_dump($res);
	$hmn = array();
	reset($res);
	$tmpk = null;
	$tmpv = array();
	while(list($qid, $arr) = each($res)) {
		$key = null;
		$val = null;
		if (strstr($qid, '_')) {
			// rank or other
			list($qid, $sub) = explode('_', $qid);
			if ($arr[0] != 8)
				continue; // other
			// rank
			$key = $arr[1];
			if ($arr[3] < 0)
				$arr[3] = 'n/a';
			else
				$arr[3]++;
			$val = $arr[2] . ' = ' . $arr[3];
			if ($tmpk != $key) {
				if (!empty($tmpk) || count($tmpv))
					array_push($hmn, array($tmpk, $tmpv));
				$tmpk = null;
				$tmpv = array();
			}
			$tmpk = $key;
			array_push($tmpv, $val);
			continue;
		}
		if (!empty($tmpk) || count($tmpv))
			array_push($hmn, array($tmpk, $tmpv));
		if (is_array($arr[0])) {
			// mutiple
			$key = $arr[0][1];
			$val = array();
			foreach ($arr as $subarr) {
				if (ereg("^!other", $subarr[2])) {
					$tmpv = preg_replace(array("/^!other=/","/^!other/"),
							array('', 'Other'), $subarr[2]);
					$tmp = preg_replace("/^other/", $qid, $subarr[3]);
					if (isset($res[$tmp]))
						$tmpv .= ': '. $res[$tmp][2];
					array_push($val, $tmpv);
				} else {
					array_push($val, $subarr[2]);
				}
			}
		} else {
			$key = $arr[1];
			if (ereg("^!other", $arr[2])) {
				$val = preg_replace(array("/^!other=/","/^!other/"),
						array('', 'Other'), $arr[2]);
				$tmp = preg_replace("/^other/", $qid, $arr[3]);
				if (isset($res[$tmp]))
					$val .= ': '. $res[$tmp][2];
			} else {
				$val = $arr[2];
			}
		}
		$tmpk = null;
		$tmpv = array();
		$hmn[] = array($key, $val);
	}
	if (!empty($tmpk) || count($tmpv))
		array_push($hmn, array($tmpk, $tmpv));
	return $hmn;
}
/* }}} */

/* {{{ proto array response_select_compact(int survey_id, int response_id, array question_ids)
   A wrapper around response_select(), that returns an array of
   key/value pairs more suitable for computer parsing.
 */
function response_select_compact($sid, $rid, $qids = null) {
	$res = response_select($sid, $rid, 'type_id', $qids);
	$cpq = array();
	reset($res);
	while(list($qid, $arr) = each($res)) {
		if (strstr($qid, '_')) {
			// rank or other
			if ($arr[0] == 8) {
				// rank
				$cpq[] = array($qid, $arr[2], array($arr[1]));
			} else {
				// other
				$cpq[] = array($qid, $arr[1]);
			}
		} elseif (is_array($arr[0])) {
			// multiple
			$cpq[] = array($qid,
					array_map(create_function('$a', 'return $a[2];'), $arr),
					array_map(create_function('$b', 'return $b[1];'), $arr));
		} else {
			if ($arr[0] == 4 || $arr[0] == 6)
				$cpq[] = array($qid, $arr[2], array($arr[1]));
			else
				$cpq[] = array($qid, $arr[2]);
		}
	}
	return $cpq;
}
/* }}} */

/* {{{ proto array response_select_name(int survey_id, int response_id, array question_ids)
   A wrapper around response_select(), that returns an array of
   key/value pairs using the field name as the key.
 */
function response_select_name($sid, $rid, $qids = null) {
	$res = response_select($sid, $rid, 'type_id,name', $qids);
	$nam = array();
	reset($res);
	while(list($qid, $arr) = each($res)) {
		$key = null;
		$val = null;
		if (strstr($qid, '_')) {
			// rank or other
			list($qid, $sub) = explode('_', $qid);
			if ($arr[0] != 8)
				continue; // other

			// rank
			$str1 = $arr[2];
			do {
				$str2 = $str1;
				$str1 = eregi_replace(
					"(^| )(what|which|why|how|who|where|how|is|are|were|the|a|it|of|do|you|your|please|enter)[ ?]",
					" ", $str2);
			} while ($str1 != $str2);
			$str1 = trim(strtoupper(eregi_replace(
				"[^A-Z0-9]+", " ", $str1)));
			$str1 = ereg_replace(' +','_',$str1);
			$arr[1] .= "_$str1";
			$nam[$arr[1]] = $arr[3];
			continue;
		}
		if (is_array($arr[0])) {
			// mutiple
			$key = $arr[0][1];
			$val = array();
			foreach ($arr as $subarr) {
				if (ereg("^!other", $subarr[2])) {
					$tmpv = preg_replace(array("/^!other=/","/^!other/"),
							array('', 'Other'), $subarr[2]);
					$tmp = preg_replace("/^other/", $qid, $subarr[3]);
					if (isset($res[$tmp]))
						$tmpv .= ': '. $res[$tmp][2];
					array_push($val, $tmpv);
				} else {
					array_push($val, $subarr[2]);
				}
			}
		} else {
			$key = $arr[1];
			if (ereg("^!other", $arr[2])) {
				$val = preg_replace(array("/^!other=/","/^!other/"),
						array('', 'Other'), $arr[2]);
				$tmp = preg_replace("/^other/", $qid, $arr[3]);
				if (isset($res[$tmp]))
					$val .= ': '. $res[$tmp][2];
			} else {
				$val = $arr[2];
			}
		}
		$nam[$key] = $val;
	}
	return $nam;
}
/* }}} */
//
// Add function by Y.Sakai
//
function send_email_with_attach($from,$to,$subject,$body,$headers,$url,$fnm) {
	$enc_charset = $GLOBALS['ESPCONFIG']['enc_charset'];	//"iso-2022-jp";
	if ($GLOBALS['ESPCONFIG']['default_lang'] == 'ja_JP' && function_exists('mb_encode_mimeheader')){
		$subject = mb_encode_mimeheader( $subject, $enc_charset, "B" );
	}
	$attach_flag = FALSE;
	if( !empty( $url) ) {
		$fd = @fopen( $url, "r" );
		if( $fd ) {
			$attach_file = fread( $fd, filesize($url) );
			fclose( $fd );
			//$attach_mimetype = mime_content_type( $url );
			$attach_flag = TRUE;
		}
	}
	$boundary = "--Boundary_" . uniqid( rand() );
	$fnamedotpos = strrpos($fnm,'.');
	$fext = strtolower(substr($fnm,$fnamedotpos+1));
	switch($fext) {
		case "pdf": $attach_mimetype = "application/pdf"; break;
		case "doc": $attach_mimetype = "applicaion/msword"; break;
		case "xls": $attach_mimetype = "application/excel"; break;
		case "ppt": $attach_mimetype = "application/powerpoint"; break;
		case "jpg": case "jpeg": $attach_mimetype = "image/jpeg"; break;
		case "txt": $attach_mimetype = "text/plain"; break;
		default: $attach_mimetype = "application/octet-stream"; break;
	}
	$mimetype = ($attach_flag ? "multipart/mixed;\r\n boundary=\"$boundary\"" : "text/plane;charset=\"iso-2022-jp\"" );
	$chgheader = 
		"From: ". $from ."\r\n" . 
		"Reply-To: ". $to . "\r\n" . 
		"MIME-Version: 1.0\r\n" . 
		"Content-Type: " . $mimetype . "\r\n";
	if( $attach_flag ) {
		$body = "--" . $boundary . "\r\n" . "Content-Type: text/plain;charset = \"iso-2022-jp\"\r\n" .
			"Content-Transfer-Encoding: 7bit\r\n\r\n" .
			mb_convert_encoding( $body, $enc_charset, "auto" ) . "\r\n\r\n" .
			"--" . $boundary . "\r\n" .
			"Content-Type: " . $attach_mimetype . ";\r\n\tname=\"".$fnm."\"\r\n" .
			"Content-Transfer-Encoding: base64\r\n" .
			"Content-Disposition: attachment;\r\n\tfilename=\"".$fnm."\"\r\n\r\n" .
			chunk_split(base64_encode( $attach_file )) .
			"\r\n" .
			"--" . $boundary . "--\r\n";
	} else {
		if (function_exists('mb_convert_encoding')){
			$body = mb_convert_encoding( $body, $enc_charset, "auto" );
		}
	}
	return mail($to,$subject,$body,$chgheader);
}
// End by Y.Sakai

/* {{{ proto bool response_send_email(int survey_id, int response_id)
   Retrieves the response from the database and sends an email with the
   values.  Returns true if mail sent.
 */
function response_send_email($sid, $rid) {
	global $xoopsUser, $xoopsDB;

	if (!$GLOBALS['ESPCONFIG']['allow_email']) {
		return true;
	}

	$sql = "SELECT name,title,email FROM ".TABLE_SURVEY." WHERE id='${sid}'";
	$result = mysql_query($sql);
	list($name, $title,$email) = mysql_fetch_row($result);
	mysql_free_result($result);

	if(empty($email))
		return(false);

	//lets check to see if user wants human readable email
	if ($GLOBALS['ESPCONFIG']['human_email']) {
		$answers = response_select_human($sid, $rid," ORDER BY q.position");
		$qsep = " : ";
		$isep = ",";
		$end = "\n";
	} else {
		$answers = response_select_compact($sid, $rid);
		$qsep = ' : ';
		$isep = ',';
		$end = "\n";
	}
	if($xoopsUser){
		$userHander = new XoopsUserHandler($xoopsDB);
		$tUser = $userHander->get($xoopsUser->uid());
		$uname = $tUser->uname();
		$from =  $tUser->email();
	}else{
		$uname = "Anonymous";
		$from = $email;
	}
	$user = array('survey.id' => $sid);
	$subject = 'S'.$sid.'R'.$rid.':'.$title.','.$uname;	//mb_(_MD_MAIL_TITLE) ." $name [$rid]";
	$body = '';
	reset($user);
	while (list($k, $v) = each($user))
//		$body .= $k . $qsep . $v . $end;
	$headers = "From: \"phpESP ".
		addslashes($GLOBALS['ESPCONFIG']['version']) .
		"\" <phpesp@". $GLOBALS['HTTP_SERVER_VARS']['SERVER_NAME'] .">\n";
	$headers .= "X-Sender: <phpesp@". $GLOBALS['HTTP_SERVER_VARS']['SERVER_NAME'] .">\n";
	$headers .= "X-Mailer: phpESP\n";
	$headers .= "Return-Path: <". $GLOBALS['HTTP_SERVER_VARS']['SERVER_ADMIN'] ."@".
		$GLOBALS['HTTP_SERVER_VARS']['SERVER_NAME'] . ">\n";

	reset($answers);
	while($arr = array_shift($answers)) {
		unset($x);
		if (count($arr) > 2)
			list($k, $v, $x) = $arr;
		else
			list($k, $v) = $arr;
		if (is_array($v))
			$v = implode($isep, $v);
		if (isset($x)) {
			if (is_array($x))
				$v .= ' (' . implode($isep, $x) . ')';
			else
				$v .= ' = ' . $x;
		}
		if (!eregi("img src=|a href=",$v))
			$body .= $k . $qsep . $v . $end;
		// Added by Y.Sakai
		$url='';
		$fnm='';
		if ( eregi("img src=|a href=",$v) ){
			$u = split("[<,>]",$v);
			$url = ereg_replace("img src=|a href=","",$u[1]);
			$url = ereg_replace("target='_blank'","",$url);
			$url = ereg_replace("'","",$url);
			$url = cnv_mbstr(rawurldecode($url));
			$fnm = $u[2];
		}
		// End Added
	}
	//print("url=$url,fnm=$fnm");
	//return(mail($email,$subject,$body,$headers));
	return(send_email_with_attach($from,$email,$subject,$body,$headers,$url,$fnm));	// Modified by Y.Sakai
}
/* }}} */

/* {{{ proto int response_select_max_pos(int survey_id, int response_id)
   Returns the position of the last answered question in a response.
 */
function response_select_max_pos($sid, $rid) {
	$max = 0;
	
	foreach (array('bool', 'single', 'multiple', 'rank', 'text', 'other', 'date') as $tbl) {
		$sql = "SELECT MAX(q.position) FROM ".TABLE_."response_$tbl a, question q
				WHERE a.response_id = '$rid' AND
					q.id = a.question_id AND
					q.survey_id = '$sid' AND
					q.deleted = 'N'";
		$res = mysql_query($sql);
		if (($num = mysql_result($res, 0, 0)) > $max)
			$max = $num;
		mysql_free_result($res);
	}
		return $max;
}
/* }}} */

/* {{{ proto int response_select_max_pos(int survey_id, int response_id)
   Returns the number of the section in which questions have been
   answered in a response.
 */
function response_select_max_sec($sid, $rid) {
	$pos = response_select_max_pos($sid, $rid);
	$sql = "SELECT COUNT(*)+1 FROM ".TABLE_QUESTION." q
			WHERE q.survey_id = '$sid' AND
				q.type_id = 99 AND
				q.position < '$pos' AND
				q.deleted = 'N'";
	$res = mysql_query($sql);
	$max = mysql_result($res, 0, 0);
	mysql_free_result($res);
	return $max;
}
/* }}} */

/* {{{ proto void response_import_sec(int survey_id, int response_id, int section, &array destination)
   Populates the destination array with the answers from a given
   section of a given response.
 */
function response_import_sec($sid, $rid, $sec, $varr = null) {
	if ($varr == null)
		$varr =& $GLOBALS['HTTP_POST_VARS'];
	
	$ids = survey_get_sections($sid);
	if ($sec < 1 || !isset($ids[$sec - 1]))
		return;
	$vals = response_select($sid, $rid, 'content', $ids[$sec - 1]);

	reset($vals);
	foreach ($vals as $id => $arr) {
		if (isset($arr[0]) && is_array($arr[0])) {
			// multiple
			$varr[$id] = array_map('array_pop', $arr);
		} else {
			$varr[$id] = array_pop($arr);
		}
	}	
}
/* }}} */

?>