<?php
/**
 * @file
 * Template for the detail of a production.
 */
?>

<h2><?php print $title; ?></h2>

beoordelingen:
<?php if ($comment_count > 0): ?>
  <span class="comments"><?php print $comment_count; ?></span>
  <a href="#lees">Lees beoordelingen</a>
  <a href="#schrijf">Schrijf een beoordeling</a>
<?php else: ?>
  <span class="no-comments"><?php print $comment_count; ?></span>
  <a href="#schrijf">Schrijf als eerste een beoordeling</a>
<?php endif; ?>

<?php if ($themes): ?>
Thema: <?php print $themes[0] ?><br/>
<?php endif; ?>

Korte beschrijving: <?php print $shortdescription; ?><br/>

<dl class="clearfix">

  <?php if ($location): ?>
  <dt>Waar</dt>
  <dd>
    <?php print $location['title'];?><br/>
    <?php if (!empty($location['street'])): ?>
    <?php print $location['street'] ?>, <?php print $location['zip']; ?> <?php print $location['city']; ?>
    <?php endif; ?>
    <?php if (!empty($regions)): ?>
    <ul>
    <?php foreach ($regions as $region): ?>
      <li><?php print $region?></li>
    <?php endforeach; ?>
    </ul>
    <?php endif; ?>
  </dd>
  <?php endif; ?>

  <?php if (!empty($when)): ?>
  <dt>Wanneer</dt>
  <dd><?php print $when; ?></dd>
  <?php endif; ?>

  <?php if (!empty($educationlevels)): ?>
  <dt>Niveau</dt>
  <dd>
    <ul>
    <?php foreach ($educationlevels as $educationlevel): ?>
      <li><?php print $educationlevel; ?></li>
    <?php endforeach; ?>
    </ul>
  </dd>
  <?php endif; ?>

  <?php if (!empty($themes)): ?>
  <dt>Thema</dt>
    <dd>
      <ul>
      <?php foreach ($themes as $theme): ?>
        <li><?php print $theme; ?></li>
      <?php endforeach; ?>
      </ul>
    </dd>
  <? endif; ?>

  <?php if (!empty($links)): ?>
  <dt>Links</dt>
  <dd>
    <ul>
    <?php foreach ($links as $link): ?>
      <li><?php print $link; ?></li>
    <?php endforeach; ?>
    </ul>
  </dd>
  <?php endif; ?>

</dl>

<img src="<?php print $main_picture; ?>" />

<?php foreach ($pictures as $picture): ?>
  <img src="<?php print $picture; ?>?width=160&height=120&crop=auto" />
<?php endforeach; ?>