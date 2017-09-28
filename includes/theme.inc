<?php

/**
 * @file
 * Theming functions for Barnard Digital Collections.
 */

/**
 * Generates themed newspaper issue navigator output.
 *
 * @see theme_islandora_newspaper_issue_navigator()
 */
function theme_bc_islandora_newspaper_issue_navigator(&$vars) {
  module_load_include('inc', 'islandora_newspaper', 'includes/utilities');
  $output = '';
  $object = $vars['object'];
  if ($page_links = _bc_islandora_np_page_pager($object)) {
    $output .= "<p><strong>Pages: </strong>{$page_links}</p>";
  }
  if ($dl_links = _bc_islandora_dl_links($object)) {
    $output .= '<p><strong>Download: </strong>' . $dl_links . '</p>';
  }
  $newspaper = islandora_newspaper_get_newspaper($object);
  $newspaper = $newspaper ? islandora_object_load($newspaper) : FALSE;
  $issues = $newspaper ? islandora_newspaper_get_issues($newspaper) : [];
  ksort($issues);
  $issues = array_keys($issues);
  if (empty($issues)) {
    $issues[] = $object->id;
  }
  $index = array_search($object->id, $issues);
  $previous_issue = isset($issues[$index - 1]) ? $issues[$index - 1] : NULL;
  $next_issue = isset($issues[$index + 1]) ? $issues[$index + 1] : NULL;
  $links = [];
  if (isset($issues[$index - 1])) {
    $previous_issue = $issues[$index - 1];
    $links[] = [
      'title' => t('Previous issue'),
      'href' => url("islandora/object/{$previous_issue}", ['absolute' => TRUE]),
    ];
  }
  if (isset($issues[$index + 1])) {
    $next_issue = $issues[$index + 1];
    $links[] = [
      'title' => t('Next issue'),
      'href' => url("islandora/object/{$next_issue}", ['absolute' => TRUE]),
    ];
  }
  if ($newspaper) {
    $links[] = [
      'title' => t('All issues'),
      'href' => url('bulletin', ['absolute' => TRUE]),
    ];
  }
  $attributes = ['class' => ['links', 'inline']];
  $output .= theme('links', ['links' => $links, 'attributes' => $attributes]);

  return $output;
}

/**
 * Generates themed newspaper page controls output.
 *
 * @see theme_islandora_newspaper_page_controls()
 */
function theme_bc_islandora_newspaper_page_controls(&$vars) {
  module_load_include('inc', 'islandora', 'includes/datastream');
  module_load_include('inc', 'islandora', 'includes/utilities');
  global $base_url;
  $object = $vars['object'];
  $output = '';
  $page_pager = _bc_islandora_np_page_pager($object);
  if (!empty($page_pager)) {
    $output .= '<p><strong>Pages:</strong>&nbsp;';
    $output .= $page_pager;
    $output .= '</p>';
  }

  if ($dl_links = _bc_islandora_dl_links($object)) {
    $output .= '<p><strong>Download: </strong>' . $dl_links . '</p>';
  }

  return $output;
}

/**
 * Generates themed breadcrumb output.
 *
 * @param array $vars
 *   Drupal page array.
 *
 * @return string
 *   Themed breadcrumbs.
 */
function theme_bc_islandora_breadcrumb(array &$vars) {
  $ret = '';
  $home_link = l(t('Home'), '/');

  // All breadcrumbs begin with a link to go "Home".
  if (!in_array($home_link, $vars['breadcrumb'])) {
    $bc = [$home_link];
  }
  else {
    $bc = $vars['breadcrumb'];
  }

  // Barnard Islandora Object Breadcrumb Handler.
  if (arg(2) && $object = islandora_object_load(arg(2))) {
    $menu_active_title = menu_get_active_title();
    // @TODO investigate further, please. why might this be in err? what is wrong?
    $parent_islandora_collection = islandora_get_parents_from_rels_ext($object);
    $parent_islandora_collection = isset($parent_islandora_collection[0]) ? $parent_islandora_collection[0] : 'NORELSPARENT';

    // The first* breadcrumb for any Islandora Object is "Collection".
    // (*not including 'home' or any predicate breadcrumbs.)
    $bc[] = l(t('Collections'), 'collections');

    // We rely on the local identifier from MODS to map our (Barnard's) Record
    // Groups to objects, and therefore the collections they belong to.
    // @see: https://archives.barnard.edu/finding-materials/collections-list
    //
    // Note we later switch to a PID structure that follows our collection list
    // but continue to rely on the MOD's local identifier for backwards compat.
    $mods = isset($object['MODS']) ? simplexml_load_string($object['MODS']->getContent(NULL)) : NULL;
    if ($mods) {
      $mods_local_identifier = explode('_', (string) $mods->identifier);
      $local_record_group_id = preg_replace('/^BC/', '', array_shift($mods_local_identifier));
      switch ($local_record_group_id) {

        // Mortarboard.
        case '12-01':
          $bc[] = l(t('Yearbook'), 'yearbook');
          $target = variable_get('bc_islandora_yearbook_pid', 'islandora:bookCollection');
          $crumb_link = _bc_islandora_crumb_from_object($parent_islandora_collection, $target);
          if ($crumb_link) {
            $bc[] = $crumb_link;
          }
          $bc[] = _bc_islandora_crumb_trunc($menu_active_title);
          break;

        // Student Publications.
        // Breadcrumbs are slightly different here than from Yearbooks.
        // @DEPRECATED, rationale: one generator breadcrumb to rule them all.
        // "Special" student publications: don't provide issue link.
        // 12-09, The Torch, 1959.
        // 12-16, Why We Strike, 1970.
        // 12-29, QZine.
        // 12-29, QZine.
        // 12-30, Plimpton Cookbook.
        case '12-04':
        case '12-05':
        case '12-06':
        case '12-09':
        case '12-11':
        case '12-14':
        case '12-17':
        case '12-18':
        case '12-22':
        case '12-23':
        case '12-24':
        case '12-26':
        case '12-29':
        case '12-30':
        case '12-31':
          $bc[] = l(t('Student Publications'), 'student-publications');
          $target = variable_get('bc_islandora_pubs_pid', 'islandora:1022');
          $crumb_link = _bc_islandora_crumb_from_object($parent_islandora_collection, $target);
          if ($crumb_link) {
            $bc[] = $crumb_link;
            // What does crumb trunc do? by default it explodes on commas and
            // will return that last part. e.g., "Page 3".
            $bc[] = _bc_islandora_crumb_trunc($menu_active_title);
          }
          else {
            $bc[] = $menu_active_title;
          }
          break;

        // 12-03, Bulletin.
        // @todo: bring this inline with the rest.
        case '12-03':
          module_load_include('inc', 'islandora_newspaper', 'includes/utilities');
          $bc[] = l(t('Newspaper'), 'bulletin');
          // Is it a page? If so, link to parent issue.
          $issue_pid = islandora_newspaper_get_issue($object);
          if ($issue = islandora_object_load($issue_pid)) {
            $bc[] = l($issue->label, "islandora/object/{$issue->id}");
          }
          break;

        // Barnard Center for Research on Women (BC13-58).
        case '13-58':
          $target = variable_get('bc_islandora_manuscripts_pid', 'islandora:manuscriptCollection');
          $bc[] = l(t('BCRW'), 'bcrw');
          $crumb_link = _bc_islandora_crumb_from_object($parent_islandora_collection, $target);
          if ($crumb_link) {
            $bc[] = $crumb_link;
            $bc[] = _bc_islandora_crumb_trunc($menu_active_title);
          }
          else {
            $bc[] = $menu_active_title;
          }
          break;

        // Barnard Alum Scrapbook and Diary Collection (BC15).
        case '15-02':
        case '15-04':
        case '15-05':
        case '15-06':
        case '15-07':
        case '15-08':
        case '15-09':
        case '15-11':
        case '15-12':
        case '15-18':
        case '15-20':
        case '15-28':
          $bc[] = l(t('Alum Scrapbooks'), 'scrapbooks');

          $target = "BC$local_record_group_id:0";
          $alumna_collection = islandora_object_load($target);
          $alumna_collection_crumb = _bc_islandora_crumb_from_object($alumna_collection, $object->id);

          if (empty($alumna_collection_crumb)) {
            $bc[] = $alumna_collection->label;
            break;
          }
          else {
            $bc[] = $alumna_collection_crumb;
          }

          $parent = $object->relationships->get(FEDORA_RELS_EXT_URI, 'isConstituentOf');
          if ($parent) {
            $parent = islandora_object_load($parent[0]['object']['value']);
            $parent = _bc_islandora_crumb_from_object($parent, $target, TRUE, 2);
            $bc[] = $parent;
          }
          $bc[] = _bc_islandora_crumb_trunc($menu_active_title, 2);
          break;

        default:
          $genre = $mods && isset($mods->genre) ? (string) $mods->genre : NULL;

          // Handle photographs.
          // Though photographs is typically Barnard's MODS->genre[0], we
          // need to account for other possibilities.
          // This array contains those possibilities: add as necessary.
          // @SEE: http://islandora:8080/solr/<core>/select?q=RELS_EXT_hasModel_uri_s%3A%22info%3Afedora%2Fislandora%3Asp_large_image_cmodel%22&wt=xml&indent=true&facet=true&facet.field=mods_genre_ms&rows=0
          $photograph_mods_genres = [
            'photographs',
            'black-and-white photographs',
            'portraits',
            'color photographs',
          ];

          if (in_array($genre, $photograph_mods_genres)) {
            $bc[] = l(t('Photographs'), 'photographs');
          }

          // If not a photo, generate links based on relatedItem metadata.
          else {
            $links = [];
            foreach ($mods->relatedItem as $ri) {
              if ($host_title = (string) $ri->titleInfo->title) {
                $bc_link = l($host_title, 'islandora/search', [
                  'query' => [
                    'type' => 'dismax',
                    'f[0]' => 'mods_relatedItem_host_titleInfo_title_ms' . ':"' . $host_title . '"',
                  ],
                ]);
              }
              // Record group.
              if (isset($ri['displayLabel']) && $ri['displayLabel'] == 'Record Group') {
                $links['rg'] = $bc_link;
              }
              // Collection.
              elseif (isset($bc_link)) {
                $links['c'] = $bc_link;
              }
            }
            // Record group precedes collection in breadcrumb sequence.
            if (isset($links['rg'])) {
              $bc[] = $links['rg'];
            }
            if (isset($links['c'])) {
              $bc[] = $links['c'];
            }
          }
          break;
      }
    }
  }
  // Barnard Islandora Exhibit Module Breadcrumb Handler.
  elseif (arg(0) == 'node' && is_numeric(arg(1))) {
    $node = menu_get_object();
    if ($node->type == 'exhibition') {
      $bc[] = l(t('Exhibits'), 'exhibits');
    }
  }

  $ret .= "<div class='breadcrumb'>" . implode(' » ', $bc) . "</div>";

  return $ret;
}

/**
 * Provides a link to an object but never a link to itself.
 *
 * @param \AbstractObject $object
 *   The object to create a link for. This ID is compared against $target.
 * @param string $target
 *   The ID of the parent collection or compound object.
 * @param bool $truncate
 *   Should the returned link be truncated for space saving?
 * @param int $trun_len
 *   Length of the truncation to perform on the link before returning it.
 *
 * @return null|string
 *   NULL if the object's ID is a match for the target, meaning it would be a
 *   replication of a previous breadcrumb (e.g., a collection breadcrumb).
 *   Themed link otherwise.
 *
 * @logical_errors_have_been_made
 * Not going to lie that the logic behind this piece of code is rather backward.
 * I think this is a realization of some backward capability I'm attempting to
 * avoid, but what it is, exactly, I'm not sure.
 */
function _bc_islandora_crumb_from_object($object, $target, $truncate = FALSE, $trun_len = 1) {
  // To provide assurances we don't C&B.  There is no type hint.
  if (!is_object($object)) {
    return NULL;
  }
  if ($object->id !== $target) {
    $parent_label = $object->label;
    $label = $truncate ? _bc_islandora_crumb_trunc($parent_label, $trun_len) : $parent_label;
    $link = "islandora/object/{$object->id}";
    return l(t($label), $link);
  }
  return NULL;
}

/**
 * Helper function returning truncated portions off the end of strings.
 *
 * @param string $string
 *   Incoming string to be truncated from the end.
 * @param int $range
 *   Truncation length from the end of the string in words.
 *
 * @return string
 *   Truncated string of $range, with first letter capitalized.
 */
function _bc_islandora_crumb_trunc($string, $range = 1) {
  $title_parts = explode(',', $string);
  $ret = array_slice($title_parts, -$range, $range);
  return ucfirst(trim(implode($ret)));
}

/**
 * Helper function to generate download links for given datastreams.
 */
function _bc_islandora_dl_links($obj, $datastreams = ['PDF', 'JPG']) {
  module_load_include('inc', 'islandora', 'includes/datastream');
  $urls = [];
  foreach ($datastreams as $ds) {
    if (isset($obj[$ds])) {
      $ds_label = $obj[$ds]->label;
      $dl_url = islandora_datastream_get_url($obj[$ds], 'download');
      $urls[] = l($ds_label, $dl_url);
    }
    elseif ($ds == 'TRANSCRIPT') {
      $ts_path = "islandora/object/{$obj->id}/download_transcript";
      // We set alias = TRUE here to avoid generating an aliased URL.
      // @see https://api.drupal.org/api/drupal/includes%21common.inc/function/url/7.x
      $urls[] = l(t('Transcript'), $ts_path, ['alias' => TRUE]);
    }
  }

  return implode('<span class="separator">&nbsp;|&nbsp;</span>', $urls);
}

/**
 * Helper function to determine whether $object is a textual document.
 *
 * Called by barnard_theme's template.php.
 */
function _bc_islandora_is_manuscript($object) {
  module_load_include('inc', 'islandora_paged_content', 'includes/utilities');
  $output = '';
  if (is_object($object->relationships)) {
    $results = $object->relationships->get(FEDORA_RELS_EXT_URI, 'isMemberOfCollection');
    if (!empty($results) && $parent_collection = $results[0]['object']['value']) {
      $collection = islandora_object_load($parent_collection);
      // If this object's parent collection's pid is the same as our database
      // variable bc_islandora_manuscripts_pid, the answer is YES.
      if ($collection->id == variable_get('bc_islandora_manuscripts_pid',
          'islandora:manuscriptCollection')
      ) {
        return TRUE;
      }
    }
  }

  return FALSE;
}

/**
 * Helper function to get the sequential position of an object.
 *
 * Called by barnard_theme's template.php.
 */
function _bc_islandora_get_sequence($object) {
  $seq_results = $object->relationships->get(ISLANDORA_RELS_EXT_URI, 'isSequenceNumber');
  if (!empty($seq_results)) {
    return $seq_results[0]['object']['value'];
  }

  return NULL;
}

/**
 * Helper function to generate newspaper pager.
 */
function _bc_islandora_np_page_pager($object) {
  module_load_include('inc', 'islandora_paged_content', 'includes/utilities');

  $results = $object->relationships->get(ISLANDORA_RELS_EXT_URI, 'isPageOf');
  $result = reset($results);
  $is_parent = $result ? FALSE : TRUE;
  $parent = $result ? islandora_object_load($result['object']['value']) : islandora_object_load($object);
  $pages = $parent ? islandora_paged_content_get_pages($parent) : [];
  $pages = array_keys($pages);

  if (empty($pages)) {
    // The page object pid.
    $pages[] = $object->id;
  }
  // The index of the current page in the issue.
  $index = array_search($object->id, $pages);
  $page_links = [];
  foreach ($pages as $i => $pid) {
    $title = $i + 1;
    // Output just a title if we're at the current page.
    if ($index !== FALSE && $i == $index) {
      $page_links[] = $title;
    }
    // Otherwise output a link.
    else {
      $page_links[] = l($title, "islandora/object/{$pid}");
    }
  }
  $page_links[] = $is_parent ? 'View All' : l('View All', "islandora/object/{$parent->id}");

  return implode($page_links, '&nbsp;');
}

/**
 * Generates themed featured object output for the front page.
 *
 * Called by barnard_theme's template.php.
 */
function _bc_islandora_featured() {
  $output = '';
  // Get a random Featured Object node.
  $node_q = db_select('node', 'n')
    ->fields('n', ['vid'])
    ->condition('type', 'featured_object')
    ->condition('status', 1)
    ->range(0, 1)
    ->orderRandom();
  if ($vid = $node_q->execute()->fetchField()) {
    // Get featured object image.
    $img_q = db_select('file_managed', 'fm')
      ->fields('fm', ['uri']);
    $img_q->join('field_data_field_object_image', 'foi', 'fm.fid = foi.field_object_image_fid');
    $img_q->condition('foi.revision_id', $vid, '=');
    if ($img_uri = $img_q->execute()->fetchField()) {
      drupal_add_js(['featured_img_path' => file_create_url($img_uri)], 'setting');
    }
    // Get caption.
    $cap_q = db_select('field_data_field_caption', 'c')
      ->fields('c', ['field_caption_value'])
      ->condition('revision_id', $vid, '=')
      ->condition('bundle', 'featured_object', '=');
    if ($cap = $cap_q->execute()->fetchField()) {
      // Get pid. !
      $pid_q = db_select('field_data_field_fedora_pid', 'fp')
        ->fields('fp', ['field_fedora_pid_value'])
        ->condition('revision_id', $vid, '=')
        ->condition('bundle', 'featured_object', '=');
      if ($pid = $pid_q->execute()->fetchField()) {
        if ((bool) parse_url($pid, PHP_URL_SCHEME)) {
          $output .= l($cap, "{$pid}");
        }
        else {
          $output .= l($cap, "islandora/object/{$pid}");
        }
      }
      else {
        $output .= $cap;
      }
    }
  }

  return $output;
}
