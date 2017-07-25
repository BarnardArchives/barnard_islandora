<?php

/**
 * @file
 * Theming functions for Barnard Digital Collections.
 */

/**
 * Implements hook_preprocess_HOOK().
 */
function bc_islandora_preprocess_islandora_compound_prev_next(array &$variables) {
  // I'm not 100% sure, but since "we" always create a compound obj and add the
  // first object as the "parent".
  $variables['themed_siblings'][0]['class'][] = 'parent';

  // This next block of code simply parses the label of the object
  // (e.g.
  foreach ($variables['themed_siblings'] as $key => &$vars) {
    if (preg_match("/(page.(\d{1,3})).*(inclusion.(\d{1,3}))/i", $vars['label'], $matches)) {
      $classes = [
        'inclusion-object',
        "inclusion-page-{$matches[2]}",
        "inclusion_page-{$matches[2]}-sequence-{$matches[4]}",
      ];
      $vars['class'] = array_merge($vars['class'], $classes);
      $vars['label'] = ucfirst($matches[0]);
      $vars['inclusion_page'] = $matches[2];
    }

    if ($variables['sequence'] > 1 && isset($vars['class'][0]) && $vars['class'][0] === 'active') {
      $variables['current_inclusion'] = $vars['inclusion_page'];
      $variables['themed_siblings'][0]['return_page'] = $vars['inclusion_page'];
    }
  }
}

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

/**
 * Generates themed breadcrumb output.
 */
function theme_bc_islandora_breadcrumb(&$vars) {
  $output = '';
  $home_link = l(t('Home'), '/');

  // All breadcrumbs begin with a link to go "Home".
  if (!in_array($home_link, $vars['breadcrumb'])) {
    $bc = [$home_link];
  }
  else {
    $bc = $vars['breadcrumb'];
  }

  // Is this an islandora object?
  if (arg(2) && $object = islandora_object_load(arg(2))) {
    // Scrapbook Record Group is BC15.0.
    $object_is_scrapbook = FALSE;

    $bc_manuscript_title = '';
    $bc[] = l(t('Collections'), 'collections');
    $collection_results = $object->relationships->get(FEDORA_RELS_EXT_URI, 'isMemberOf');
    $cm_results = $object->relationships->get(FEDORA_MODEL_URI, 'hasModel');
    $mods = isset($object['MODS']) ? simplexml_load_string($object['MODS']->getContent(NULL)) : NULL;

    if ($mods) {
      $mods_local_identifier = explode('_', (string) $mods->identifier);
      $local_record_group_id = preg_replace('/^BC/', '', array_shift($mods_local_identifier));
      switch ($local_record_group_id) {

        // Mortarboard.
        case '12-01':
          $bc[] = l(t('Yearbook'), 'yearbook');
          // Is it a page (= "member of" a book, not just book collection)?
          // If so, link to the parent book.
          if ($book_link = _bc_islandora_book_link($object,
            variable_get('bc_islandora_yearbook_pid', 'islandora:bookCollection'))
          ) {
            $bc[] = $book_link;
          }
          break;

        // Student Publications.
        // 12-17, Emanon.
        // 12-31, Well-Woman Newsletter.
        case '12-04':
        case '12-05':
        case '12-06':
        case '12-11':
        case '12-14':
        case '12-17':
        case '12-18':
        case '12-22':
        case '12-23':
        case '12-24':
        case '12-31':
          $bc[] = l(t('Student Publications'), 'student-publications');
          $is_pub = TRUE;
          if ($pub_link = _bc_islandora_book_link($object, variable_get('bc_islandora_pubs_pid',
            'islandora:1022'))
          ) {
            $bc[] = $pub_link;
          }
          break;

        // "Special" student publications: don't provide issue link.
        // 12-09, The Torch, 1959.
        // 12-16, Why We Strike, 1970.
        // 12-29, QZine.
        // 12-29, QZine.
        // 12-30, Plimpton Cookbook.
        case '12-09':
        case '12-26':
        case '12-29':
        case '12-30':
          $bc[] = l(t('Student Publications'), 'student-publications');
          if ($cm_results[0]['object']['value'] == 'islandora:pageCModel') {
            $bc[] = _bc_islandora_book_link($object, variable_get('bc_islandora_pubs_pid',
              'islandora:1022'));
          }
          break;

        // 12-03, Bulletin.
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
          $bc[] = l(t('BCRW'), '/collections/bcrw');
          if ($cm_results[0]['object']['value'] == 'islandora:manuscriptPageCModel') {
            $bc_manuscript_title = _bc_islandora_book_link($object,
              variable_get('bc_islandora_manuscripts_pid', 'islandora:manuscriptCollection'));
          }
          break;

        // Barnard Alumn Scrapbook and Diary Collection (BC15).
        case '15-12':
        case '15-14':
          $object_is_scrapbook = TRUE;
          //          $title = menu_get_active_title();
          //          if (preg_match_all("/(page.\d{1,3}) | (inclusion.\d{1,3})/ix", $title, $matches)) {
          //            dpm($matches[0]);
          //          }
          $bc[] = l(t('Alumn Scrapbooks'), 'collections/scrapbooks');
          // Always provide a link to the parent collection if we aren't on it.
          if ($cm_results[0]['object']['value'] !== 'islandora:collectionCModel') {
            $alumn_parent_collection = 'BC' . $local_record_group_id . ':0';
            if ($parent = islandora_object_load($alumn_parent_collection)) {
              $bc[] = l(t($parent->label), "islandora/object/$alumn_parent_collection");
            }
          }
          if ($cm_results[0]['object']['value'] === 'islandora:pageCModel') {
            $bc[] = _bc_islandora_book_link($object, variable_get('bc_islandora_scrapbooks_pid',
              'BC15:0'));
          }
          //          else {
          //            $bc[] = l(t("Update the title."), "islandora/object/$alumn_parent_collection");
          //          }
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

  // Is this an exhibition node?
  elseif (arg(0) == 'node' && is_numeric(arg(1))) {
    $node = menu_get_object();
    if ($node->type == 'exhibition') {
      $bc[] = l(t('Exhibits'), 'exhibits');
    }
  }

  // Add the current page's title to the existing breadcrumb.
  if (count($bc) >= 1 && $title = menu_get_active_title()) {
    $title_parts = explode(',', $title);
    $last_part = array_pop($title_parts);
    // In these special cases, use whatever text occurs after the last comma
    // (e.g. "issue 1") as the title.
    $mods_local_identifier = explode(' ', preg_replace('/^ /', '', $last_part));
    if (array_shift($mods_local_identifier) == 'page'
      || !empty($is_pub) && (bool) $is_pub
    ) {
      if (!empty($bc_manuscript_title)) {
        $bc[] = $bc_manuscript_title;
      }
      $bc[] = $last_part;
    }
    else {
      $bc[] = $title . '!';
    }
  }

  $output .= '<div class="breadcrumb">' . implode(' » ', $bc) . '</div>';

  return $output;
}

/**
 * Provides a link to a parent book object.
 */
function _bc_islandora_book_link($object, $parent_collection) {
  $parent = $object->relationships->get(FEDORA_RELS_EXT_URI, 'isMemberOf');
  $parentCollection = $object->relationships->get(FEDORA_RELS_EXT_URI, 'isMemberOfCollection');

  $book = !empty($parent) ?
    islandora_object_load($parent[0]['object']['value']) :
    islandora_object_load($parentCollection[0]['object']['value']);

  // Don't generate a link to islandora:bookCollection or whatever.
  if ($book->id != $parent_collection) {
    return l($book->label, "islandora/object/{$book->id}");
  }

  return NULL;
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
