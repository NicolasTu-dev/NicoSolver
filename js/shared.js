// Affiliate link tracking: ?ref=codigo gets remembered so the purchase
// on cuenta.html still counts for that streamer even if the visitor
// browses the rest of the landing first.
(function(){
  const ref = new URLSearchParams(window.location.search).get('ref');
  if(ref) localStorage.setItem('solverix_ref', ref.trim().toLowerCase());
})();
