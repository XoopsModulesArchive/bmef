<?php

# $Id: survey_update.inc,v 1.12 2003/01/31 04:02:47 jimmerman Exp $

// Written by James Flemer
// For eGrad2000.com
// <jflemer@alum.rpi.edu>

/* {{{ proto bool survey_update(int* survey_id, string* tab, string old_tab)
   Reads current form variables from HTTP_POST_VARS.
   Returns an true on sucess, else returns false and
   sets global $errstr with an error message. */
function survey_update(&$survey_id, &$tab, $old_tab) {
	global $HTTP_POST_VARS, $HTTP_SERVER_VARS, $errstr;

	// do not need update
	if(empty($old_tab))
		return(1);

	$f_arr = array();
	$v_arr = array();

	// new survey
	if(empty($survey_id)) {
		if (isset($HTTP_POST_VARS['name'])) {
		$HTTP_POST_VARS['name'] = eregi_replace(
			"[^A-Z0-9]+", "_", trim($HTTP_POST_VARS['name']) );
		$HTTP_POST_VARS['name'] = ereg_replace('_$',"",$HTTP_POST_VARS['name']);
		}

		// need to fill out at least some info on 1st tab before proceeding
		if(empty($HTTP_POST_VARS['name']) || empty($HTTP_POST_VARS['title'])
				|| empty($HTTP_POST_VARS['realm'])) {
			$tab = "general";
			$errstr = mb_('Sorry, please fill out the name, group, and title before proceeding.');
			return(0);
		}

		// create a new survey in the database
		$fields = array('name','realm','title','subtitle','email','theme','thanks_page','thank_head','thank_body','info');
		foreach($fields as $f) {
			if(isset($HTTP_POST_VARS[$f])) {
				array_push($f_arr,$f);
				array_push($v_arr,"'"._addslashes($HTTP_POST_VARS[$f])."'");
			}
		}
		array_push($f_arr,'owner');
		array_push($v_arr,"'".$GLOBALS['HTTP_SESSION_VARS']['acl']['username']."'");
		$sql = "INSERT INTO ".TABLE_SURVEY." (" . join(',',$f_arr) . ") VALUES (" . join(',',$v_arr) . ")";

		$result = @mysql_query ($sql);
		if(!$result) {
			$tab = "general";
			$errstr = mb_('Sorry, name already in use. Pick a new name.') .' [ ' .mysql_errno().': '.mysql_error().' ]';
			return(0);
		}

		$sql = "SELECT id FROM ".TABLE_SURVEY." WHERE name='".  _addslashes($HTTP_POST_VARS['name']) ."'";
		$result = mysql_query($sql);
		$survey_id = mysql_result($result,0,0);
		mysql_free_result($result);
		return(1);
	}

	// survey already started
	switch($old_tab) {
		// coming from the general tab ...
		case "general":
			if (isset($HTTP_POST_VARS['name'])) {
			$HTTP_POST_VARS['name'] = eregi_replace(
				"[^A-Z0-9]+", "_", trim($HTTP_POST_VARS['name']) );
			$HTTP_POST_VARS['name'] = ereg_replace('_$',"",$HTTP_POST_VARS['name']);
			}

			if(empty($HTTP_POST_VARS['name']) || empty($HTTP_POST_VARS['title'])
					|| empty($HTTP_POST_VARS['realm'])) {
				$tab = "general";
				$errstr = mb_('Sorry, please fill out the name, group, and title before proceeding.');
				return(0);
			}
			$fields = array('name','realm','title','subtitle','email','theme','thanks_page','thank_head','thank_body','info');
			$sql = "SELECT name FROM ".TABLE_SURVEY." WHERE id='${survey_id}'";
			$result = mysql_query($sql);
			$name = mysql_result($result,0,0);
			mysql_free_result($result);

			// trying to change survey name
			if($name != $HTTP_POST_VARS['name']) {
				$sql = "SELECT COUNT(*) FROM ".TABLE_SURVEY." WHERE name='" . _addslashes($HTTP_POST_VARS['name']) ."'";
				$result = mysql_query($sql);
				$count = mysql_result($result,0,0);
				mysql_free_result($result);
				if($count != 0) {
					$tab = "general";
					$errstr = mb_('Sorry, that name is already in use.');
					return(0);
				}
			}

			// UPDATE the row in the DB with current values
			foreach($fields as $f) {
				array_push($f_arr,$f ."='" . _addslashes($HTTP_POST_VARS[$f]) . "'");
			}
			$sql = "UPDATE ".TABLE_SURVEY." SET " . join(', ',$f_arr) . " WHERE id='${survey_id}'";
			$result = mysql_query($sql);
			if(!$result) {
				$tab = "general";
				$errstr = mb_('Warning, error encountered.') .' [ '.mysql_errno().': '.mysql_error().' ]';
				return(0);
			}
			return(1);

		// coming from the questions tab ...
		case "questions":
			// if the question box is empty ... ignore everything else
			if(empty($HTTP_POST_VARS['content']) && empty($HTTP_POST_VARS['name']))
				return(1);

			if(empty($HTTP_POST_VARS['content'])) {
				$tab = 'questions';
				$dont_clear = 1;
				$errstr = mb_('Please enter text for this question.');
				return(0);
			}

			// constraint: fieldname must be not empty
			//   generate it from the content if empty
			//   validate/repair fieldname
			if(empty($HTTP_POST_VARS['name'])) {
				$str1 = $HTTP_POST_VARS['content'];
				do {
					$str2 = $str1;
					$str1 = eregi_replace(
					"(^| )(what|which|why|how|who|where|how|is|are|were|the|a|it|of|do|you|your|please|enter)[ ?]",
					" ", $str2);
				} while ($str1 != $str2);
				$HTTP_POST_VARS['name'] = $str1;
			}
			$HTTP_POST_VARS['name'] = strtoupper(substr( eregi_replace(
				"[^A-Z0-9]+", "_", trim($HTTP_POST_VARS['name'])), 0, 30));
			$HTTP_POST_VARS['name'] = ereg_replace('_$',"",$HTTP_POST_VARS['name']);

			// constraint: question type required
			if(empty($HTTP_POST_VARS['type_id'])) {
				$tab = 'questions';
				$dont_clear = 1;
				$errstr= mb_('Sorry, you must select a type for this question.');
				return(0);
			}

			// constraint: qid must be int or empty
			if($HTTP_POST_VARS['id'] == mb_('New Field'))
				$qid = '';
			$qid = intval($HTTP_POST_VARS['id']);	# curr_q_id

			// constraint: can not change between question w/ answer choices and one w/o
			$has_choices = esp_type_has_choices();
			if(!empty($qid)) {
				$sql =  "SELECT Q.type_id FROM ".TABLE_QUESTION." Q WHERE Q.survey_id='${survey_id}' AND Q.id='${qid}'";
				$result = mysql_query($sql);
				$old_type_id = mysql_result($result,0,0);
				mysql_free_result($result);
				if($has_choices[$HTTP_POST_VARS['type_id']] != $has_choices[$old_type_id]) { // trying to change between incompatible question types
					$tab = "questions";
					$HTTP_POST_VARS['type_id'] = $old_type_id;
					$dont_clear = 1;
					$errstr = mb_('Sorry, you cannot change between those types of question. Create a new question instead.');
					return(0);
				}
			}

			// constraint: length must be int
			$HTTP_POST_VARS['length']  = intval($HTTP_POST_VARS['length']) or 0;

			// constraint: precise must be int
			$HTTP_POST_VARS['precise'] = intval($HTTP_POST_VARS['precise']) or 0;

			// defaults for length field
			if(empty($HTTP_POST_VARS['length']) && $HTTP_POST_VARS['type_id'] < 50) {
				$arr = array(
					0,		// 0: unused
					0,		// 1: Yes/No
					20,		// 2: Text Box  (width)
					60,		// 3: Essay Box (width)
					0,		// 4: Radio Buttons
					0,		// 5: Check Boxes (minumum)
					0,		// 6: Dropdown Box (length)
					5,		// 7: Rating (# cols)
					5,		// 8: Rate (# cols)
					0,		// 9: Date
					10		// 10: Numeric (digits)
					);
				$HTTP_POST_VARS['length'] = $arr[$HTTP_POST_VARS['type_id']];
			}

			// defaults for precision field
			if(empty($HTTP_POST_VARS['precise']) && $HTTP_POST_VARS['type_id'] < 50) {
				$arr = array(
					0,		// 0: unused
					0,		// 1: Yes/No
					0,		// 2: Text Box
					5,		// 3: Essay Box (height)
					10,		// 4: Radio Buttons
					0,		// 5: Check Boxes (maximum)
					0,		// 6: Dropdown Box
					0,		// 7: Rating (use N/A)
					0,		// 8: Rate (use N/A)
					0,		// 9: Date
					0		// 10: Numeric (decimal)
					);
				$HTTP_POST_VARS['precise'] = $arr[$HTTP_POST_VARS['type_id']];
			}

			// UPDATE row in the DB for the current question
			if($qid != '') {
				$fields = array('name','type_id','length','precise','required','content');
				foreach($fields as $f) {
					if(isset($HTTP_POST_VARS[$f]))
						array_push($f_arr,$f."='"._addslashes($HTTP_POST_VARS[$f])."'");
				}
				$sql = "UPDATE ".TABLE_QUESTION." SET " . join(', ',$f_arr) . " WHERE id='${qid}'";

			// INSERT row in the DB for new question
			} else {
				// set the position to the end
				$sql = "SELECT MAX(position)+1 FROM ".TABLE_QUESTION." WHERE survey_id='${survey_id}'";
				$result = mysql_query($sql);
				$HTTP_POST_VARS['position'] = mysql_result($result,0,0);
				mysql_free_result($result);

				$HTTP_POST_VARS['survey_id'] = $survey_id;

				$fields = array('survey_id','name','type_id','length','precise','required','content','position');
				foreach($fields as $f) {
					if(isset($HTTP_POST_VARS[$f])) {
						array_push($f_arr, $f);
						array_push($v_arr, "'". _addslashes($HTTP_POST_VARS[$f]) ."'");
					}
				}
				$sql = "INSERT INTO ".TABLE_QUESTION." (" . join(', ',$f_arr) . ") VALUES (" . join(', ',$v_arr) .")";
			}
			$result = mysql_query($sql);
			if($qid == '')
				$qid = mysql_insert_id();
			$HTTP_POST_VARS['id'] = $qid;
			if(!$result) {
				$tab = 'questions';
				$dont_clear = 1;
				$errstr = mb_('Warning, error encountered.') .' [ '.mysql_errno().': '.mysql_error().' ]';
				return(0);
			}

			// UPDATE or INSERT rows for each of the question choices for this question
			if($has_choices[$HTTP_POST_VARS['type_id']]) {
				$cids = array();
				$sql = "SELECT c.id FROM ".TABLE_QUESTION." q, ".TABLE_QUESTION_CHOICE." c WHERE q.id=c.question_id AND q.survey_id=${survey_id}";
				$result = mysql_query($sql);
				for ($i = 0; $i < mysql_num_rows($result); ++$i) {
					array_push($cids, mysql_result($result, $i, 0));
				}
				mysql_free_result($result);
				$count = 0;
				for($i=1;$i<$HTTP_POST_VARS['num_choices']+1;$i++) {
					$sql='';
					$choice_id      = intval($HTTP_POST_VARS["choice_id_${i}"]);
					$choice_content = _addslashes($HTTP_POST_VARS["choice_content_${i}"]);
					// each of the submitted choices
					if($choice_id=='' && $choice_content!='') {
						// insert (new)
						$sql = "INSERT INTO ".TABLE_QUESTION_CHOICE." (question_id,content) VALUES ('${qid}','${choice_content}')";
						++$count;
					} elseif($choice_id!='' && $choice_content=='') {
						// delete (old)
						$sql = "DELETE FROM ".TABLE_QUESTION_CHOICE." WHERE id='${choice_id}'";
					} elseif($choice_id!='' && $choice_content!='' && in_array($choice_id, $cids)) {
						// update (old)
						$sql = "UPDATE ".TABLE_QUESTION_CHOICE." SET content='${choice_content}' WHERE id='${choice_id}'";
						++$count;
					}
					if($sql != '') {
						$result = mysql_query($sql);
						if(!$result) {
							$tab = 'questions';
							$dont_clear = 1;
							$errstr = mb_('Warning, error encountered.') .' [ '.mysql_errno().': '.mysql_error().' ]';
							return(0);
						}
					}
				}
				if(!$count && !isset($HTTP_POST_VARS['extra_choices'])) {
					$tab = 'questions';
					$dont_clear = 1;
					$errstr = mb_('Sorry, you need at least one answer option for this question type.') .
						' [ '. mb_('ID') .': '. $HTTP_POST_VARS['type_id'] .' ]';
					return(0);
				}
			}
			return(1);

		case "preview":
			// can not change anything here yet, so no need to update DB.
			return(1);

		case "order":
			// it updates the DB itself
			return(1);
	}
	return(1);
}
/* }}} */

?>