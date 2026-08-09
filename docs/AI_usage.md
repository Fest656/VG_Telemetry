# How AI was used in this project

This project started out with the objective of helping me learn embedded programming and Windows internals. As such, I decided I wanted to try to replicate Gemini's Oriented Learning mode in agent form. In Oriented Learning mode, Gemini takes in, for example, a question and then guides you through the solution by asking you to name the variables in the problems, asking you what formulas you should use and correcting you, and then taking the problem step by step.
I achieved this with a simple prompt in which I specified that I wanted it to act as a sort of teacher that could guide me along solutions. This way I would do my own research into documentation and code examples, mostly using Microsoft Learn. Then when I ran into issues I could discuss them with the agent, whether it be about documentation, design decisions, hardware selection, etc...

## The interaction loop when coding

When developing a new function, I would start by defining it in the header file along with a comment documenting it. I would then ask the agent if it thought the function was well defined and commented, if it was i would then say that I researched this and that, how I wanted to write it and then use it, then the AI would give me feedback on my pseudo-implementation and I would go on to write it, then asking for feedback once the code was actually written.

## Starter prompt

Act as a mentor and technical advisor for the project in this directory.
Never write full code solutions. If code is genuinely needed to illustrate a concept, use short pseudocode or short snippets.
When I hit a problem, ask me questions first to see what I've already tried or considered, before offering direction.
Give guidance in the form of: concepts to look up, questions to ask myself, tradeoffs to weigh, or the shape of an approach.
Point out what I might be missing or getting wrong, but let me figure out the fix.
Treat this like an advisor relationship: you've done this before, I haven't, and your goal is for me to leave understanding why, not just having a working result.
Assume I want to struggle productively, ask insightful questions that then will lead to an easier step-by-step implementation.
