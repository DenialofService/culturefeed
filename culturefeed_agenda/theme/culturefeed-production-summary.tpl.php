<?php
/**
 * @file
 * Template for the summary of a production.
 */
?>

<div class="event-teaser">

  <h2><a href="<?print $link ?>">production => <?print $title; ?></a></h2>

  <div class="activity-wrapper">
    <div class="comment-wrapper">
      <?php if ($comment_count > 0): ?>
        <span class="comments"><?php print $comment_count; ?></span>
        <a href="<?php print $link ?>#lees">Lees beoordelingen</a>
        <a href="<?php print $link ?>#schrijf">Schrijf een beoordeling</a>
      <?php else: ?>
        <span class="no-comments"><?php print $comment_count; ?></span>
        <a href="<?php print $link ?>#schrijf">Schrijf als eerste een beoordeling</a>
      <?php endif; ?>
    </div>
    <?php if ($recommend_count > 0): ?>
      <div class="count-aangeraden"><?php print format_plural($recommend_count, '<span>@count</span> keer aangeraden', '<span>@count</span> keer aangeraden'); ?></div>
    <?php endif; ?>
    <?php print culturefeed_search_ui_activity_recommend_link($item); ?>
  </div>

  <div class="image">
    <?php if (!empty($thumbnail)): ?>
    <img src="<?php print $thumbnail; ?>?width=160&height=120&crop=auto" />
    <?php endif; ?>
  </div>

  <dl class="clearfix">

    <?php if ($location): ?>
    <dt>Waar</dt>
    <dd><?php print $location['title'] ?></dd>
    <?php endif; ?>

    <?php if (!empty($when)): ?>
    <dt>Wanneer</dt>
    <dd><?php print $when; ?></dd>
    <?php endif; ?>

    <?php
      if (!empty($themes)):
    ?>
    <dt>Thema</dt>
      <dd>
      <ul>
      <?php foreach ($themes as $theme): ?>
        <li><?php print $theme; ?></li>
      <?php endforeach; ?>
      </ul>
      </dd>
    <? endif; ?>
  </dl>

  <?php print culturefeed_search_detail_l('event', $cdbid, $title, 'Meer info en boeking', array('attributes' => array('class' => 'button'))); ?>

</div>